#!/usr/bin/env python3

# Call $0 <file with a list of component file paths>
# Dumps all the implementing constructors to stdout

import xml.sax
import sys

constructors = list()

class ComponentHandler(xml.sax.ContentHandler):
    def startElement(self, tag, attributes):
        if tag == "implementation" and "constructor" in attributes:
            constructors.append(attributes["constructor"])

if __name__ == "__main__":
    parser = xml.sax.make_parser()
    parser.setFeature(xml.sax.handler.feature_namespaces, 0)
    parser.setContentHandler(ComponentHandler())
    for filename in sys.argv[1:]:
        with open(filename, "r") as components_listfile:
            for line in components_listfile:
                for component_filename in line.strip().split():
                    parser.parse(component_filename)
    constructors.sort()
    print("\n".join(constructors))
