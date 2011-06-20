from PropertyNames import PropertyNames
from Helper import *
import traceback
import uno
'''
This class gives access to the OO configuration api.
It contains 4 get and 4 set convenience methods for getting and settings
properties in the configuration. <br/>
For the get methods, two parameters must be given: name and parent, where
name is the name of the property, parent is a HierarchyElement
(::com::sun::star::configuration::HierarchyElement)<br/>
The get and set methods support hieryrchical property names like
"options/gridX". <br/>
NOTE: not yet supported, but sometime later,
If you will ommit the "parent" parameter, then the "name" parameter must be
in hierarchy form from the root of the registry.
'''

class Configuration(object):

    @classmethod
    def getNode(self, name, parent):
        return parent.getByName(name)

    @classmethod
    def Set(self, value, name, parent):
        parent.setHierarchicalPropertyValue(name, value)

    '''
    @param name
    @param parent
    @return
    @throws Exception
    '''

    @classmethod
    def getConfigurationNode(self, name, parent):
        return parent.getByName(name)

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

    @classmethod
    def getChildrenNames(self, configView):
        return configView.getElementNames()

    @classmethod
    def getProductName(self, xMSF):
        try:
            oProdNameAccess = self.getConfigurationRoot(xMSF,
                "org.openoffice.Setup/Product", False)
            ProductName = Helper.getUnoObjectbyName(oProdNameAccess, "ooName")
            return ProductName
        except Exception:
            traceback.print_exc()
            return None

    @classmethod
    def getOfficeLocaleString(self, xMSF):
        sLocale = ""
        try:
            aLocLocale = Locale.Locale()
            oMasterKey = self.getConfigurationRoot(xMSF,
                "org.openoffice.Setup/L10N/", False)
            sLocale = (String)
            Helper.getUnoObjectbyName(oMasterKey, "ooLocale")
        except Exception, exception:
            traceback.print_exc()

        return sLocale

    @classmethod
    def getOfficeLocale(self, xMSF):
        aLocLocale = Locale.Locale()
        sLocale = getOfficeLocaleString(xMSF)
        sLocaleList = JavaTools.ArrayoutofString(sLocale, "-")
        aLocLocale.Language = sLocaleList[0]
        if sLocaleList.length > 1:
            aLocLocale.Country = sLocaleList[1]

        return aLocLocale

    @classmethod
    def getOfficeLinguistic(self, xMSF):
        try:
            oMasterKey = self.getConfigurationRoot(xMSF,
                "org.openoffice.Setup/L10N/", False)
            sLinguistic = Helper.getUnoObjectbyName(oMasterKey, "ooLocale")
            return sLinguistic
        except Exception, exception:
            traceback.print_exc()
            return None

    '''
    This method creates a new configuration node and adds it
    to the given view. Note that if a node with the given name
    already exists it will be completely removed from
    the configuration.
    @param configView
    @param name
    @return the new created configuration node.
    @throws com.sun.star.lang.WrappedTargetException
    @throws ElementExistException
    @throws NoSuchElementException
    @throws com.sun.star.uno.Exception
    '''

    @classmethod
    def addConfigNode(self, configView, name):
        if configView is None:
            return configView.getByName(name)
        else:
            print configView
            # insert it - this also names the element
            try:
                configView.insertByName(name, configView.createInstance())
            except Exception,e:
                traceback.print_exc()
            #return newNode

    @classmethod
    def removeNode(self, configView, name):

        if configView.hasByName(name):
            configView.removeByName(name)

    @classmethod
    def commit(self, configView):
        configView.commitChanges()

    @classmethod
    def updateConfiguration(self, xmsf, path, name, node, param):
        view = self.getConfigurationRoot(xmsf, path, True)
        addConfigNode(path, name)
        node.writeConfiguration(view, param)
        view.commitChanges()

    @classmethod
    def removeNode(self, xmsf, path, name):
        view = self.getConfigurationRoot(xmsf, path, True)
        removeNode(view, name)
        view.commitChanges()

    @classmethod
    def getNodeDisplayNames(self, _xNameAccessNode):
        snames = None
        return getNodeChildNames(_xNameAccessNode,
            PropertyNames.PROPERTY_NAME)

    @classmethod
    def getNodeChildNames(self, xNameAccessNode, _schildname):
        snames = None
        try:
            snames = xNameAccessNode.getElementNames()
            sdisplaynames = range(snames.length)
            i = 0
            while i < snames.length:
                oContent = Helper.getUnoPropertyValue(
                    xNameAccessNode.getByName(snames[i]), _schildname)
                if not AnyConverter.isVoid(oContent):
                    sdisplaynames[i] = (String)
                    Helper.getUnoPropertyValue(xNameAccessNode.getByName(
                        snames[i]), _schildname)
                else:
                    sdisplaynames[i] = snames[i]

                i += 1
            return sdisplaynames
        except Exception, e:
            traceback.print_exc()
            return snames

    @classmethod
    def getChildNodebyIndex(self, _xNameAccess, _index):
        try:
            snames = _xNameAccess.getElementNames()
            oNode = _xNameAccess.getByName(snames[_index])
            return oNode
        except Exception, e:
            traceback.print_exc()
            return None

    @classmethod
    def getChildNodebyName(self, _xNameAccessNode, _SubNodeName):
        try:
            if _xNameAccessNode.hasByName(_SubNodeName):
                return _xNameAccessNode.getByName(_SubNodeName)

        except Exception, e:
            traceback.print_exc()

        return None

    @classmethod
    def getChildNodebyDisplayName(self, _xNameAccessNode, _displayname):
        snames = None
        return getChildNodebyDisplayName(_xNameAccessNode, _displayname,
            PropertyNames.PROPERTY_NAME)

    @classmethod
    def getChildNodebyDisplayName(self, _xNameAccessNode, _displayname,
        _nodename):

        snames = None
        try:
            snames = _xNameAccessNode.getElementNames()
            sdisplaynames = range(snames.length)
            i = 0
            while i < snames.length:
                curdisplayname = Helper.getUnoPropertyValue(
                    _xNameAccessNode.getByName(snames[i]), _nodename)
                if curdisplayname.equals(_displayname):
                    return _xNameAccessNode.getByName(snames[i])

                i += 1
        except Exception, e:
            traceback.print_exc()

        return None

    @classmethod
    def getChildNodebyDisplayName(self, _xMSF, _aLocale, _xNameAccessNode,
        _displayname, _nodename, _nmaxcharcount):

        snames = None
        try:
            snames = _xNameAccessNode.getElementNames()
            sdisplaynames = range(snames.length)
            i = 0
            while i < snames.length:
                curdisplayname = Helper.getUnoPropertyValue(
                    _xNameAccessNode.getByName(snames[i]), _nodename)
                if (_nmaxcharcount > 0) and (_nmaxcharcount < \
                        curdisplayname.length()):
                    curdisplayname = curdisplayname.substring(0,
                        _nmaxcharcount)

                curdisplayname = Desktop.removeSpecialCharacters(_xMSF,
                    _aLocale, curdisplayname)
                if curdisplayname.equals(_displayname):
                    return _xNameAccessNode.getByName(snames[i])

                i += 1
        except Exception, e:
            traceback.print_exc()

        return None

