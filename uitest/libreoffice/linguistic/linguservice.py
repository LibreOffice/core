# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#


def get_lingu_service_manager(xContext):
    """ Returns the com.sun.star.linguistic2.LinguServiceManager

        Further information: https://api.libreoffice.org/docs/idl/ref/servicecom_1_1sun_1_1star_1_1linguistic2_1_1LinguServiceManager.html
    """
    xServiceManager = xContext.getServiceManager()
    xLinguServiceManager = xServiceManager.createInstanceWithContext("com.sun.star.linguistic2.LinguServiceManager", xContext)
    return xLinguServiceManager


def get_spellchecker(xContext):
    """ Returns the com.sun.star.linguistic2.XSpellChecker through the
    com.sun.star.linguistic2.LinguServiceManager

    Further information: https://api.libreoffice.org/docs/idl/ref/servicecom_1_1sun_1_1star_1_1linguistic2_1_1SpellChecker.html"""
    xLinguServiceManager = get_lingu_service_manager(xContext)
    return xLinguServiceManager.getSpellChecker()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
