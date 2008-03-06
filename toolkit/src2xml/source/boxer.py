
from globals import *


class DlgLayoutBuilder(object):
    def __init__ (self, dlgnode):
        self.dlgnode = dlgnode
        self.rows = {}

    def addWidget (self, elem):
        x, y = int(elem.getAttr('x')), int(elem.getAttr('y'))
        if self.rows.has_key(y):
            self.rows[y][x] = elem
        else:
            self.rows[y] = {}
            self.rows[y][x] = elem

    def build (self):
        root = Element('vbox')
        ys = self.rows.keys()
        ys.sort()
        for y in ys:
            xs = self.rows[y].keys()
            xs.sort()

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
