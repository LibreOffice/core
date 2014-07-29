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
    if url in list(namespaceAliases.keys()):
        return namespaceAliases[url]
    else:
        if url.startswith("http://"):
            return url.replace('http://', '').replace('/', '_').replace('.', '_')
        else:
            return ""


def prefixForGrammar(namespace):
    ns = nsForGrammar(namespace)
    if ns in list(namespaceAliases.keys()):
        prefix = namespaceAliases[ns]
        return prefix
    else:
        return prefixFromUrl(ns)


def nsForGrammar(namespace):
    return namespace.getElementsByTagName("grammar")[0].getAttribute("ns")


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
    sock.close()


def defaultNamespaceAliases():
    return {
        'xml': 'http://www.w3.org/XML/1998/namespace',
    }


def preprocess(model):
    for i in model.getElementsByTagName("namespace-alias"):
        name = i.getAttribute("name")
        if name in list(ooxUrlIds.keys()):
            i.setAttribute("id", ooxUrlIds[name])
        else:
            i.setAttribute("id", ooxAliasIds[i.getAttribute("alias")])
        namespaceAliases[name] = i.getAttribute("alias")

    for i in model.getElementsByTagName("namespace"):
        # Start elements in this namespace.
        startElements = [s.getAttribute("name") for s in i.getElementsByTagName("start")]

        ns = i.getElementsByTagName("grammar")[0].getAttribute("ns")
        if ns in list(namespaceAliases.keys()):
            i.setAttribute("namespacealias", namespaceAliases[ns])
        else:
            i.setAttribute("namespacealias", "")
        if ns.startswith("http://schemas.openxmlformats.org/"):
            i.setAttribute("prefix", ns.replace('http://schemas.openxmlformats.org/', '').replace('/', '_').replace('-', '_'))
        else:
            i.setAttribute("prefix", "")

        grammarprefix = prefixForGrammar(i)
        grammarns = nsForGrammar(i)

        grammarNamespaceAliases = defaultNamespaceAliases()
        grammar = i.getElementsByTagName("grammar")[0]

        parseNamespaceAliases(grammar, grammarNamespaceAliases)

        for j in i.getElementsByTagName("element") + i.getElementsByTagName("attribute"):
            if j.localName == "attribute" and not len(j.getAttribute("name")):
                continue

            if j.parentNode.localName == "resource":
                # Outside grammar element, don't inherit namespaces defined there.
                localNamespaceAliases = defaultNamespaceAliases()
            else:
                # Inside grammar element.
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

            # ns
            ns = ""
            if ":" in j.getAttribute("name"):
                nameprefix = j.getAttribute("name").split(':')[0]
                if nameprefix in list(localNamespaceAliases.keys()):
                    ns = localNamespaceAliases[nameprefix]
            elif j.localName == "attribute":
                # if parent node is resource, then we're outside the grammar element
                if grammar.getAttribute("attributeFormDefault") == "qualified" and not j.parentNode.localName == "resource":
                    ns = grammarns
            else:
                ns = grammarns

            # localname
            if ":" in j.getAttribute("name"):
                localname = j.getAttribute("name").split(':')[1]
            elif len(j.getElementsByTagName("anyName")):
                localname = "FAST_TOKENS_END"
            else:
                localname = j.getAttribute("name")

            # set the attributes
            qname = ""
            if len(ns):
                qname += ns + ":"
            j.setAttribute("qname", qname + localname)

            j.setAttribute("prefix", prefix)
            j.setAttribute("localname", localname)

    for i in model.getElementsByTagName("grammar"):
        if i.getAttribute("ns").startswith("http://schemas.openxmlformats.org/"):
            i.setAttribute("application", i.getAttribute("ns").replace('http://schemas.openxmlformats.org/', '').split('/')[0])
        else:
            i.setAttribute("application", "")


namespacesPath = sys.argv[1]
modelPath = sys.argv[2]

# URL -> ID, from oox
ooxUrlIds = {}
# Alias -> ID, from oox
ooxAliasIds = {}
parseNamespaces(namespacesPath)

# URL -> alias
namespaceAliases = {}
model = minidom.parse(modelPath)
preprocess(model)
model.writexml(sys.stdout)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
