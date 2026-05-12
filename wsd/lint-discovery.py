#!/usr/bin/env python3
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# Makes sure that discovery.xml in online.git is in sync with
# filter/source/config/fragments/ in core.git.

from __future__ import print_function
import os
import sys
import xml.sax


class DiscoveryHandler(xml.sax.handler.ContentHandler):
    """Parses an online.git discovery.xml."""

    def __init__(self):
        # Dict of app -> {extension -> action}
        self.appActions = {}
        self.app = None
        # A dict of 'extension, (actions)'
        self.allExtensions = dict()

    def startElement(self, name, attrs):
        if name == "app":
            for k, v in list(attrs.items()):
                if k == "name":
                    self.app = v
                    self.appActions[self.app] = {}
        elif name == "action" and self.app:
            action = None
            ext = None
            for k, v in list(attrs.items()):
                if k == "name":
                    action = v
                elif k == "ext":
                    ext = v
            if action and ext:
                self.appActions[self.app][ext] = action
                if ext in self.allExtensions:
                    # Potential problem:
                    # see 2de5017e329ce09efbd8f4dc6066fdba3e2c080c
                    # discovery.xml with duplicating "ext" is valid, but
                    # can't be used directly in e.g. SharePoint, unless
                    # specific extensions are imported using
                    # New-SPWOPIBinding's parameters, avoiding
                    # the duplication.
                    # Note 2026: we register them with two different actions.
                    if action not in self.allExtensions[ext]:
                        print("info: Registering extension '{}' for {} with a different action '{}' (current: '{}'.".format(ext, self.app, action, self.allExtensions[ext]))
                        self.allExtensions[ext].append(action)
                    else:
                        print("warning: extension '" + ext +
                              "' exists for '" + self.app + "', " +
                              "but already used earlier in discovery.xml")
                else:
                    self.allExtensions[ext] = [action]

    def endElement(self, name):
        if name == "app" and self.app:
            self.app = None


class FilterTypeHandler(xml.sax.handler.ContentHandler):
    """Parses core.git filter/source/config/fragments/types/*.xcu."""

    def __init__(self):
        self.name = None
        self.inExtensions = False
        self.content = []
        self.extensions = None
        self.extensionsSep = " "

    def startElement(self, name, attrs):
        if name == "node":
            for k, v in list(attrs.items()):
                if k == "oor:name":
                    self.name = v
        elif name == "prop":
            for k, v in list(attrs.items()):
                if k == "oor:name" and v == "Extensions":
                    self.inExtensions = True
        elif name == "value" and self.inExtensions:
            for k, v in list(attrs.items()):
                if k == "oor:separator":
                    self.extensionsSep = v

    def endElement(self, name):
        if name == "prop" and self.inExtensions:
            self.inExtensions = False
            self.extensions = "".join(self.content).strip()\
                                .encode("utf-8").split(bytes(self.extensionsSep, "utf-8"))
            self.extensionsSep = " "
            self.content = []

    def characters(self, content):
        if self.inExtensions:
            self.content.append(content)


class FilterFragmentHandler(xml.sax.handler.ContentHandler):
    """Parses core.git filter/source/config/fragments/filters/*.xcu."""

    def __init__(self):
        self.inType = False
        self.typeName = None
        self.inFlags = False
        self.flags = None
        self.inDocumentService = False
        self.documentService = None
        self.content = []

    def startElement(self, name, attrs):
        if name == "prop":
            for k, v in list(attrs.items()):
                if k == "oor:name" and v == "Type":
                    self.inType = True
                elif k == "oor:name" and v == "Flags":
                    self.inFlags = True
                elif k == "oor:name" and v == "DocumentService":
                    self.inDocumentService = True

    def endElement(self, name):
        if name == "prop" and self.inType:
            self.inType = False
            self.typeName = "".join(self.content).strip()
            self.content = []
        elif name == "prop" and self.inFlags:
            self.inFlags = False
            encodedFlags = "".join(self.content).strip().encode("utf-8")
            self.flags = encodedFlags.split(b" ")
            self.content = []
        elif name == "prop" and self.inDocumentService:
            self.inDocumentService = False
            self.documentService = "".join(self.content).strip()
            self.content = []

    def characters(self, content):
        if self.inType or self.inFlags or self.inDocumentService:
            self.content.append(content)


# Builds a 'document service' -> {'extension' -> 'filter flags'} dictionary.
def getExtensionProperties(filterDir):
    # Build a 'type name' -> 'extensions' dictionary.
    typeNameExtensions = {}
    typeFragments = os.path.join(filterDir, "types")
    for typeFragment in os.listdir(typeFragments):
        if not typeFragment.endswith(".xcu"):
            continue
        parser = xml.sax.make_parser()
        filterTypeHandler = FilterTypeHandler()
        parser.setContentHandler(filterTypeHandler)
        parser.parse(os.path.join(typeFragments, typeFragment))
        # Did we find some extensions?
        if filterTypeHandler.extensions:
            typeNameExtensions[filterTypeHandler.name] = \
                filterTypeHandler.extensions
    # Build a 'type name' -> ('filter flag list', 'document service')
    # dictionary.
    typeNameFlags = {}
    filterFragments = os.path.join(filterDir, "filters")
    for filterFragment in os.listdir(filterFragments):
        if not filterFragment.endswith(".xcu"):
            continue
        parser = xml.sax.make_parser()
        handler = FilterFragmentHandler()
        parser.setContentHandler(handler)
        parser.parse(os.path.join(filterFragments, filterFragment))
        if "IMPORT" in handler.flags:
            if handler.typeName in typeNameFlags:
                if "EXPORT" in typeNameFlags[handler.typeName][0]:
                    # don't modify a filetype with maximal capabilities
                    continue
            typeNameFlags[handler.typeName] = \
                (handler.flags, handler.documentService)
    # Now build the combined
    # 'document service' -> {'extension' -> 'filter flags'}.
    extensionProperties = {}
    for typeName in typeNameExtensions:
        if typeName not in typeNameFlags:
            continue
        flags, documentService = typeNameFlags[typeName]
        if documentService not in extensionProperties:
            extensionProperties[documentService] = {}
        for extension in typeNameExtensions[typeName]:
            extensionProperties[documentService][extension] = flags
    return extensionProperties


# Map app names to document service names
appDocumentServices = {
    'writer': 'com.sun.star.text.TextDocument',
    'writer-global': 'com.sun.star.text.GlobalDocument',
    'writer-web': 'com.sun.star.text.WebDocument',
    'calc': 'com.sun.star.sheet.SpreadsheetDocument',
    'impress': 'com.sun.star.presentation.PresentationDocument',
    'draw': 'com.sun.star.drawing.DrawingDocument',
}
documentServicesApp = {v: k for k, v in appDocumentServices.items()}

# We know about these extensions
extensionsSkipList = {
    'xls',  # we know that it can be edited
    'pdf',  # it exists for draw - its entry in core.git is missing document
            # service
    'zip',
    'htm',
    'html',
    'xhtml',
    '*',   # well, obvious ;-)
    '',    # and this :-D
}


def main():
    discoveryXml = "discovery.xml"
    try:
        repoGuess = os.environ["COREPATH"]
    except:
        scriptPath = os.path.abspath(os.path.dirname(sys.argv[0]))
        repoGuess = os.path.abspath(os.path.join(scriptPath, "../engine"))
    filterDir = os.path.join(repoGuess, "filter/source/config/fragments")
    if len(sys.argv) >= 3:
        discoveryXml = sys.argv[1]
        filterDir = sys.argv[2]

    # Parse discovery.xml, which describes what online.git exposes at the
    # moment.
    parser = xml.sax.make_parser()
    discoveryHandler = DiscoveryHandler()
    parser.setContentHandler(discoveryHandler)
    parser.parse(discoveryXml)

    # Parse core.git filter definitions to build a
    # 'document service' -> {'extension' -> 'filter flags'} dictionary.
    extensionProperties = getExtensionProperties(filterDir)

    for ext, actions in discoveryHandler.allExtensions.items():
        if 'edit' in actions and not 'view' in actions:
            print(f"warning: extension '{ext}' doesn't have 'view' but has 'edit'.")

    proposed = {}

    # Now look up the filter flags in core.git for the extension.
    for app, actions in discoveryHandler.appActions.items():
        if app not in appDocumentServices:
            continue  # e.g., for "Capabilities"
        documentService = appDocumentServices[app]
        if documentService not in extensionProperties:
            # Inconsistency found.
            print("warning: actions for '" + app + "' " +
                  "exist, but not found in core.git")
            continue
        for extension, discoveryAction in actions.items():
            if extension in extensionsSkipList:
                continue
            if extension not in extensionProperties[documentService]:
                # Inconsistency found.
                print("warning: action for '" + app + ":" + extension + "' " +
                      "exists, but is not found in core.git")
                continue
            flags = extensionProperties[documentService][extension]
            if "IMPORT" in flags and "EXPORT" in flags:
                coreAction = "edit"
            else:
                coreAction = "view"

            if discoveryAction != coreAction:
                # Inconsistency found.
                print("warning: action for '" + app + ":" + extension + "' " +
                      "is '" + discoveryAction + "', " +
                      "but it should be '" + coreAction + "'")

        # Now see if there are any new extensions in the core.git filter config
        # which are missing.
        for extension, flags in extensionProperties[documentService].items():
            if extension not in actions:
                if "IMPORT" in flags and "EXPORT" in flags:
                    action = "edit"
                else:
                    action = "view"
                if app not in proposed:
                    proposed[app] = {}
                proposed[app][extension] = action

    # Now see if there are any new types in the core.git filter config which
    # are missing.
    for documentService, extensions in extensionProperties.items():
        missingName = None
        if documentService not in documentServicesApp:
            # Inconsistency found.
            print("warning: extensions for '" + documentService + "' " +
                  "found in core.git, without mapping to apps "
                  "in discovery.xml")
            missingName = documentService
        else:
            app = documentServicesApp[documentService]
            if app not in discoveryHandler.appActions:
                # Inconsistency found.
                print("warning: extensions for '" + app + "' " +
                      "found in core.git, all missing in discovery.xml")
                missingName = app

        if missingName:
            for extension, flags in extensions.items():
                if "IMPORT" in flags and "EXPORT" in flags:
                    action = "edit"
                else:
                    action = "view"
                if missingName not in proposed:
                    proposed[missingName] = {}
                proposed[missingName][extension] = action

    # Produce a copy&paste-able XML output for the proposed changes.
    for app, extensions in proposed.items():
        newExtensions = {}
        for extension, action in extensions.items():
            if extension in extensionsSkipList:
                continue
            if extension in discoveryHandler.allExtensions:
                continue  # see 2de5017e329ce09efbd8f4dc6066fdba3e2c080c
            newExtensions[extension] = action
        if not newExtensions:
            continue  # no extensions after filtering

        print('        <app name="' + app + '">')
        for extension, action in newExtensions.items():
            print('            <action name="' + action + '" default="true" ' +
                  'ext="' + extension + '"/>')
        print('        </app>')


if __name__ == "__main__":
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
