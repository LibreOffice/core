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
import traceback
import inspect
from .ConfigGroup import ConfigGroup
from .Configuration import Configuration

class ConfigSet(ConfigGroup):
    '''
    After reading the configuration set items,
    the ConfigSet checks this field.
    If it is true, it will remove any nulls from
    the vector.
    subclasses can change this field in the constructor
    to avoid this "deletion" of nulls.
    '''

    def __init__(self, topic):
        self.topic = topic
        self.childrenList = []

    def add(self, name, o):
        self.childrenList.append(o)

    def writeConfiguration(self, configView, param):
        names = self.childrenMap.keys()
        if isinstance(self.childClass, ConfigNode):
            #first I remove all the children from the configuration.
            children = configView.ElementNames
            if children:
                for i in children:
                    try:
                        Configuration.removeNode(configView, i)
                    except Exception:
                        traceback.print_exc()

                # and add them new.
            for i in names:
                try:
                    child = self.getElement(i)
                    childView = configView.getByName(i)
                    child.writeConfiguration(childView, param)
                except Exception:
                    traceback.print_exc()
        else:
            raise AttributeError (
            "Unable to write primitive sets to configuration (not implemented)")


    def readConfiguration(self, configurationView, param):
        #each iteration represents a Topic row
        names = configurationView.ElementNames
        if names:
            for i in names:
                try:
                    self.topic.readConfiguration(
                        configurationView.getByName(i), param)
                    self.add(i, self.topic)
                except Exception:
                    traceback.print_exc()
