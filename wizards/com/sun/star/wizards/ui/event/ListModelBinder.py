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

class ListModelBinder(object):

    def __init__(self, unoListBox, listModel_):
        self.unoList = unoListBox
        self.unoListModel = unoListBox.Model
        #COMMENTED
        #self.setListModel(listModel_)

    def setListModel(self, newListModel):
        if self.listModel is not None:
            self.listModel.removeListDataListener(self)

        self.listModel = newListModel
        self.listModel.addListDataListener(this)

    def contentsChanged(self, lde):
        selected = self.getSelectedItems()
        i = lde.getIndex0()
        while i <= lde.getIndex1():
            self.update(i)
            i += 1
        self.setSelectedItems(selected)

    def update(self, i):
        self.remove(i, i)
        self.insert(i)

    def remove(self, i1, i2):
        self.unoList.removeItems(i1, i2 - i1 + 1)

    def insert(self, i):
        self.unoList.addItem(self.getItemString(i), i)

    def getItemString(self, i):
        return self.getItemString(self.listModel.getElementAt(i))

    def getItemString(self, item):
        return self.renderer.render(item)

    def getSelectedItems(self):
        return self.unoListModel.SelectedItems

    def setSelectedItems(self, selected):
        self.unoListModel.SelectedItems = selected;

    def intervalAdded(self, lde):
        for i in xrange(lde.Index0, lde.Index1):
            self.insert(i)

    def intervalRemoved(self, lde):
        self.remove(lde.Index0, lde.Index1)

    @classmethod
    def fillList(self, xlist, items, renderer):
        xlist.Model.StringItemList = ()
        for index,item in enumerate(items):
            if item is not None:
                if renderer is not None:
                    aux = renderer.render(index)
                else:
                    aux = item.cp_Name
                xlist.addItem(aux, index)

    @classmethod
    def fillComboBox(self, xComboBox, items, renderer):
        xComboBox.Model.StringItemList = ()
        for index,item in enumerate(items):
            if item is not None:
                if renderer is not None:
                    aux = renderer.render(index)
                else:
                    aux = item.toString()
                xComboBox.addItem(aux, index)
