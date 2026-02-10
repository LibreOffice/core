# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import org.libreoffice.unotest
from org.libreoffice.embindtest import ImplicitConstructor
from org.libreoffice.embindtest import ExplicitConstructors
from com.sun.star.script import CannotConvertException

class ServiceConstructorsTest(unittest.TestCase):
    def test_implicit_constructor(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()
        obj = ImplicitConstructor.create(ctx)
        self.assertEqual(obj.getArguments(), tuple())

    def test_multiple_arguments(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()
        obj = ExplicitConstructors.multipleArguments(ctx, 42, " is half of ", 84.0)
        self.assertEqual(obj.getArguments(), (42, " is half of ", 84.0))

    def test_interface_argument(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()
        obj = ExplicitConstructors.interfaceArgument(ctx, ctx)
        self.assertEqual(obj.getArguments(), (ctx, ))

        # None should be coerced into a null reference even though
        # normally it’s supposed to represent the void type
        obj = ExplicitConstructors.interfaceArgument(ctx, None)
        self.assertEqual(obj.getArguments(), (None, ))

    def test_rest_arguments(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()

        obj = ExplicitConstructors.restArgument(ctx)
        self.assertEqual(obj.getArguments(), tuple())

        obj = ExplicitConstructors.restArgument(ctx, 1, 2, "buckle my shoe")
        self.assertEqual(obj.getArguments(), (1, 2, "buckle my shoe"))

    def test_invalid_arguments(self):
        ctx = org.libreoffice.unotest.pyuno.getComponentContext()

        with self.assertRaises(AttributeError) as cm:
            ImplicitConstructor.create()
        self.assertEqual(str(cm.exception),
                         "org.libreoffice.embindtest.ImplicitConstructor::create "
                         "requires 1 argument")

        with self.assertRaises(AttributeError) as cm:
            ExplicitConstructors.interfaceArgument()
        self.assertEqual(str(cm.exception),
                         "org.libreoffice.embindtest.ExplicitConstructors::interfaceArgument "
                         "requires 2 arguments")

        with self.assertRaises(AttributeError) as cm:
            ExplicitConstructors.interfaceArgument(1, 2, 3)
        self.assertEqual(str(cm.exception),
                         "org.libreoffice.embindtest.ExplicitConstructors::interfaceArgument "
                         "requires 2 arguments")

        with self.assertRaises(AttributeError) as cm:
            ExplicitConstructors.restArgument()
        self.assertEqual(str(cm.exception),
                         "org.libreoffice.embindtest.ExplicitConstructors::restArgument "
                         "requires at least 1 argument")

        with self.assertRaises(AttributeError) as cm:
            ImplicitConstructor.create(True)
        self.assertEqual(str(cm.exception),
                         "First argument to a service constructor must be an "
                         "XComponentContext")

        with self.assertRaises(CannotConvertException):
            ExplicitConstructors.interfaceArgument(ctx, 12)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
