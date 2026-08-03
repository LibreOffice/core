# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest

from org.libreoffice.unotest import pyuno
from com.sun.star.script.provider import theMasterScriptProviderFactory, ScriptURIHelper
from com.sun.star.ucb import UniversalContentBroker
from com.sun.star.uri import UriReferenceFactory
from com.sun.star.container import NoSuchElementException
import uno

import contextlib
import os
import tempfile


testEnvironmentInitialized = False


def get_user_script_directory(context, language):
    uriHelper = ScriptURIHelper.create(context, language, "user")
    uri = uriHelper.getRootStorageURI()
    return uno.fileUrlToSystemPath(uri)


class ProviderExpandAllTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.context = pyuno.getComponentContext()
        global testEnvironmentInitialized
        if not testEnvironmentInitialized:
            pyuno.private_initTestEnvironment()
            testEnvironmentInitialized = True

            # The ExpandContentProvider is needed for ScriptURIHelper to work
            xUcb = UniversalContentBroker.create(cls.context)
            xExpandProvider = cls.context.getServiceManager().createInstance(
                "com.sun.star.ucb.ExpandContentProvider")
            xUcb.registerContentProvider(xExpandProvider, "vnd.sun.star.expand", True)

    # Tries retrieving all nodes from the master script provider to make sure all of the providers
    # are accessible from Python. See tdf#172532
    def test_expand_all(self):
        # Disable the Java-based JavaScript provider so that we can be sure the QuickJS-based one
        # will be used. Otherwise a random one will end up getting chosen and if it happens to be
        # the Java one then the test will fail. The Java code is shared with the BeanShell provider
        # so that will already be tested that way.
        try:
            self.context.getServiceManager().remove(
                "com.sun.star.script.framework.provider.javascript.ScriptProviderForJavaScript$"
                "ScriptProviderForJavaScript_2")
        except NoSuchElementException:
            pass

        with contextlib.ExitStack() as stack:
            # If the QuickJS-based script provider is available then create a dummy JavaScript macro
            # to make sure the provider is triggered.
            if ("com.sun.star.script.provider.ScriptProviderForJavaScript"
                in self.context.getServiceManager().getAvailableServiceNames()):
                have_js_provider = True
                js_path = get_user_script_directory(self.context, "JavaScript")
                os.makedirs(js_path, exist_ok=True)
                js_dir = stack.enter_context(tempfile.TemporaryDirectory(dir=js_path))
                with open(os.path.join(js_dir, "dummy_js_script.js"), 'w'):
                    pass
            else:
                have_js_provider = False

            xMasterProviderFactory = theMasterScriptProviderFactory.get(self.context)

            found_js_script = False

            for provider_context in ['user', 'share']:
                xMasterProvider = xMasterProviderFactory.createScriptProvider(provider_context)

                stack = [xMasterProvider]

                while True:
                    try:
                        node = stack.pop()
                    except IndexError:
                        break

                    if node.getName() == "dummy_js_script":
                        found_js_script = True

                    # Add all of the children to the stack
                    if node.hasChildNodes():
                        stack.extend(node.getChildNodes())

            if have_js_provider:
                self.assertTrue(found_js_script)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
