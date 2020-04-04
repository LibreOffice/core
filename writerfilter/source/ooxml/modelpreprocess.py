#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from xml.dom import minidom
import sys


def prefixForGrammar(namespace):
    ns = namespace.getElementsByTagName("grammar")[0].getAttribute("ns")
    return ooxUrlAliases[ns]


def parseNamespaceAliases(node):
    ret = {}
    for k, v in list(node.attributes.items()):
        if k.startswith("xmlns:"):
            ret[k.replace('xmlns:', '')] = v
    return ret


def parseNamespaces(fro):
    sock = open(fro)
    for i in sock.readlines():
        line = i.strip()
        alias, url = line.split(' ')[1:]
        ooxUrlAliases[url] = alias
    sock.close()


def check(model):
    defines = [i.getAttribute("name") for i in model.getElementsByTagName("define")]
    for reference in [i.getAttribute("name") for i in model.getElementsByTagName("ref")]:
        if reference not in defines:
            raise Exception("Unknown define element with name '%s'" % reference)
    for start in [i.getAttribute("name") for i in model.getElementsByTagName("start")]:
        if start not in defines:
            raise Exception("Unknown start element with name '%s'" % start)


def preprocess(model):
    modelNode = [i for i in model.childNodes if i.localName == "model"][0]
    # Alias -> URL, based on "xmlns:" attributes.
    modelNamespaceAliases = parseNamespaceAliases(modelNode)
    for i in modelNode.getElementsByTagName("namespace"):
        grammarprefix = prefixForGrammar(i)

        grammar = i.getElementsByTagName("grammar")[0]

        for j in i.getElementsByTagName("element") + i.getElementsByTagName("attribute"):
            # prefix
            prefix = ""
            if ":" in j.getAttribute("name"):
                nameprefix = j.getAttribute("name").split(':')[0]
                prefix = ooxUrlAliases[modelNamespaceAliases[nameprefix]]
            elif j.localName == "attribute":
                if grammar.getAttribute("attributeFormDefault") == "qualified":
                    prefix = grammarprefix
            else:
                prefix = grammarprefix

            # localname
            if ":" in j.getAttribute("name"):
                localname = j.getAttribute("name").split(':')[1]
            else:
                localname = j.getAttribute("name")

            # set the attributes
            j.setAttribute("prefix", prefix)
            j.setAttribute("localname", localname)


namespacesPath = sys.argv[1]
modelPath = sys.argv[2]

# URL -> alias, from oox
ooxUrlAliases = {}
parseNamespaces(namespacesPath)

model = minidom.parse(modelPath)
check(model)
preprocess(model)
model.writexml(sys.stdout)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
