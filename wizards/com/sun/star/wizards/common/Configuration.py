#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import uno
import traceback
from .PropertyNames import PropertyNames
from .Helper import Helper

class Configuration(object):
    '''This class gives access to the OO configuration api.'''

    @classmethod
    def getConfigurationRoot(self, xmsf, sPath, updateable):
        oConfigProvider = xmsf.createInstance(
            "com.sun.star.configuration.ConfigurationProvider")
        args = []

        aPathArgument = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        aPathArgument.Name = "nodepath"
        aPathArgument.Value = sPath

        args.append(aPathArgument)
        if updateable:
            sView = "com.sun.star.configuration.ConfigurationUpdateAccess"
            aModeArgument = uno.createUnoStruct(
                'com.sun.star.beans.PropertyValue')
            aModeArgument.Name = "lazywrite"
            aModeArgument.Value = False
            args.append(aModeArgument)
        else:
            sView = "com.sun.star.configuration.ConfigurationAccess"

        return oConfigProvider.createInstanceWithArguments(sView, tuple(args))
