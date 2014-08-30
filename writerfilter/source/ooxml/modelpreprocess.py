#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from __future__ import print_function
from xml.dom import minidom
import sys


def prefixFromUrl(url):
    if url in list(ooxUrlAliases.keys()):
        return ooxUrlAliases[url]
    else:
        if url.startswith("http://"):
            return url.replace('http://', '').replace('/', '_').replace('.', '_')
        else:
            return ""


def prefixForGrammar(namespace):
    ns = namespace.getElementsByTagName("grammar")[0].getAttribute("ns")
    if ns in list(ooxUrlAliases.keys()):
        prefix = ooxUrlAliases[ns]
        return prefix
    else:
        return prefixFromUrl(ns)


def parseNamespaceAliases(node, ret):
    for k, v in list(node.attributes.items()):
        if k.startswith("xmlns:"):
            ret[k.replace('xmlns:', '')] = v


def parseNamespaces(fro):
    sock = open(fro)
    for i in sock.readlines():
        line = i.strip()
        id, alias, url = line.split(' ')
        ooxUrlIds[url] = id
        ooxAliasIds[alias] = id
        ooxUrlAliases[url] = alias
    sock.close()


def defaultNamespaceAliases():
    return {
        'xml': 'http://www.w3.org/XML/1998/namespace',
    }


def check(model):
    defines = [i.getAttribute("name") for i in model.getElementsByTagName("define")]
    for reference in [i.getAttribute("name") for i in model.getElementsByTagName("ref")]:
        if not reference in defines:
            raise Exception("Unknown define element with name '%s'" % reference)
    for start in [i.getAttribute("name") for i in model.getElementsByTagName("start")]:
        if not start in defines:
            raise Exception("Unknown start element with name '%s'" % start)


def preprocess(model):
    for i in model.getElementsByTagName("namespace"):
        grammarprefix = prefixForGrammar(i)

        grammarNamespaceAliases = defaultNamespaceAliases()
        grammar = i.getElementsByTagName("grammar")[0]

        parseNamespaceAliases(grammar, grammarNamespaceAliases)

        for j in i.getElementsByTagName("element") + i.getElementsByTagName("attribute"):
            if j.localName == "attribute" and not len(j.getAttribute("name")):
                continue

            localNamespaceAliases = grammarNamespaceAliases.copy()

            parseNamespaceAliases(j.parentNode, localNamespaceAliases)
            parseNamespaceAliases(j, localNamespaceAliases)

            # prefix
            prefix = ""
            if ":" in j.getAttribute("name"):
                nameprefix = j.getAttribute("name").split(':')[0]
                if nameprefix in list(localNamespaceAliases.keys()):
                    prefix = prefixFromUrl(localNamespaceAliases[nameprefix])
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

# URL -> ID, from oox
ooxUrlIds = {}
# Alias -> ID, from oox
ooxAliasIds = {}
# URL -> alias, from oox
ooxUrlAliases = {}
parseNamespaces(namespacesPath)

model = minidom.parse(modelPath)
check(model)
preprocess(model)
model.writexml(sys.stdout)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
