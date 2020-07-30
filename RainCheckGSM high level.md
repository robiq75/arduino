## File contains a drawing of high level HW modules interconnect and SW modules cooperation.

Linking flowchart:
TEST
linked from 
https://github.com/SemanticMediaWiki/Mermaid/blob/master/docs/USAGE.md

// Going to activate Mermaid 

{{#mermaid:sequenceDiagram
participant Alice
participant Bob
  Alice->John: Hello John, how are you?
  loop Healthcheck
       John->John: Fight against hypochondria
  end
  Note right of John: Rational thoughts <br/>prevail...
    John-->Alice: Great!
    John->Bob: How about you?
    Bob-->John: Jolly good!
}}
