from textx.metamodel import metamodel_from_file

ui_dsl_mm = metamodel_from_file('ui_logger_dsl_grammar.tx')

# Load the program:
actions = ui_dsl_mm.model_from_file('example.ul')

for statement in actions.commands:
    print(statement)
