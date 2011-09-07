from common.Helper import Helper

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
        selected = getSelectedItems()
        i = lde.getIndex0()
        while i <= lde.getIndex1():
            update(i)
            i += 1
        setSelectedItems(selected)

    def update(self, i):
        remove(i, i)
        insert(i)

    def remove(self, i1, i2):
        self.unoList.removeItems(i1, i2 - i1 + 1)

    def insert(self, i):
        self.unoList.addItem(getItemString(i), i)

    def getItemString(self, i):
        return getItemString(self.listModel.getElementAt(i))

    def getItemString(self, item):
        return self.renderer.render(item)

    def getSelectedItems(self):
        return Helper.getUnoPropertyValue(self.unoListModel, "SelectedItems")

    def setSelectedItems(self, selected):
        Helper.setUnoPropertyValue(self.unoListModel, "SelectedItems", selected)

    def intervalAdded(self, lde):
        for i in xrange(lde.Index0, lde.Index1):
            insert(i)

    def intervalRemoved(self, lde):
        remove(lde.Index0, lde.Index1)

    @classmethod
    def fillList(self, xlist, items, renderer):
        Helper.setUnoPropertyValue(xlist.Model, "StringItemList", ())
        for index,item in enumerate(items):
            if item is not None:
                if renderer is not None:
                    aux = renderer.render(index)
                else:
                    aux = item.cp_Name
                xlist.addItem(aux, index)

    @classmethod
    def fillComboBox(self, xComboBox, items, renderer):
        Helper.setUnoPropertyValue(xComboBox.Model, "StringItemList", ())
        for index,item in enumerate(items):
            if item is not None:
                if renderer is not None:
                    aux = renderer.render(index)
                else:
                    aux = item.toString()
                xComboBox.addItem(aux, index)
