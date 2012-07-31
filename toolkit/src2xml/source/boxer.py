# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************


from globals import *


class DlgLayoutBuilder(object):
    def __init__ (self, dlgnode):
        self.dlgnode = dlgnode
        self.rows = {}

    def addWidget (self, elem):
        x, y = int(elem.getAttr('x')), int(elem.getAttr('y'))
        self.rows[y] = self.rows.get (y, {})
        while self.rows[y].has_key(x):
            y += 1
            self.rows[y] = self.rows.get (y, {})
        self.rows[y][x] = elem

    def build (self):
        root = Element('vbox')
        ys = sorted(self.rows.keys())
        for y in ys:
            xs = sorted(self.rows[y].keys())

            if len(xs) == 1:
                root.appendChild(self.rows[y][xs[0]])
                continue

            hbox = Element('hbox')
            root.appendChild(hbox)
            for x in xs:
                elem = self.rows[y][x]
                hbox.appendChild(elem)

        return root


class Boxer(object):
    def __init__ (self, root):
        self.root = root

    def layout (self):

        newroot = RootNode()
        for dlgnode in self.root.children:
            newdlgnode = self.__walkDlgNode(dlgnode)
            newroot.children.append(newdlgnode)

        return newroot

    def __walkDlgNode (self, dlgnode):

        newnode = Element(dlgnode.name)
        newnode.clone(dlgnode)
        if dlgnode.name == 'string':
            return newnode
        newnode.setAttr("xmlns", "http://openoffice.org/2007/layout")
        newnode.setAttr("xmlns:cnt", "http://openoffice.org/2007/layout/container")
        mx = DlgLayoutBuilder(newnode)

        # Each dialog node is expected to have a flat list of widgets.
        for widget in dlgnode.children:
            if widget.hasAttr('x') and widget.hasAttr('y'):
                mx.addWidget(widget)
            else:
                newnode.appendChild(widget)

        vbox = mx.build()
        if len(vbox.children) > 0:
            newnode.appendChild(vbox)

        return newnode
