import traceback
from common.Helper import *

'''
To change the template for this generated type comment go to
Window>Preferences>Java>Code Generation>Code and Comments
'''

class PeerConfig(object):

    def __init__(self, _oUnoDialog):
        self.oUnoDialog = _oUnoDialog
        #self.oUnoDialog.xUnoDialog.addWindowListener(self)
        self.m_aPeerTasks = []
        self.aControlTasks = []
        self.aImageUrlTasks = []
        self.oUnoDialog = None

    class PeerTask(object):

        def __init__(self,_xControl, propNames_, propValues_):
            self.propnames = propNames_
            self.propvalues = propValues_
            self.xControl = _xControl

    class ControlTask(object):

        def __init__(self, _oModel, _propName, _propValue):
            self.propname = _propName
            self.propvalue = _propValue
            self.oModel = _oModel

    class ImageUrlTask(object):

        def __init__(self, _oModel , _oResource, _oHCResource):
            self.oResource = _oResource
            self.oHCResource = _oHCResource
            self.oModel = _oModel

    def windowShown(self, arg0):
        try:
            i = 0
            while i < self.m_aPeerTasks.size():
                aPeerTask = self.m_aPeerTasks.elementAt(i)
                xVclWindowPeer = aPeerTask.xControl.getPeer()
                n = 0
                while n < aPeerTask.propnames.length:
                    xVclWindowPeer.setProperty(aPeerTask.propnames[n],
                        aPeerTask.propvalues[n])
                    n += 1
                i += 1
            i = 0
            while i < self.aControlTasks.size():
                aControlTask = self.aControlTasks.elementAt(i)
                Helper.setUnoPropertyValue(aControlTask.oModel,
                    aControlTask.propname, aControlTask.propvalue)
                i += 1
            i = 0
            while i < self.aImageUrlTasks.size():
                aImageUrlTask = self.aImageUrlTasks.elementAt(i)
                sImageUrl = ""
                if isinstance(aImageUrlTask.oResource,int):
                    sImageUrl = self.oUnoDialog.getWizardImageUrl(
                        (aImageUrlTask.oResource).intValue(),
                        (aImageUrlTask.oHCResource).intValue())
                elif isinstance(aImageUrlTask.oResource,str):
                    sImageUrl = self.oUnoDialog.getImageUrl(
                        (aImageUrlTask.oResource),
                        (aImageUrlTask.oHCResource))

                if not sImageUrl.equals(""):
                    Helper.setUnoPropertyValue(
                        aImageUrlTask.oModel,
                        PropertyNames.PROPERTY_IMAGEURL, sImageUrl)

                i += 1
        except RuntimeException, re:
            traceback.print_exc
            raise re;

    '''
    @param oAPIControl an API control that the interface XControl
    can be derived from
    @param _saccessname
    '''

    def setAccessibleName(self, oAPIControl, _saccessname):
        setPeerProperties(oAPIControl, ("AccessibleName"), (_saccessname))

    def setAccessibleName(self, _xControl, _saccessname):
        setPeerProperties(_xControl, ("AccessibleName"), (_saccessname))

    '''
    @param oAPIControl an API control that the interface XControl
    can be derived from
    @param _propnames
    @param _propvalues
    '''

    def setPeerProperties(self, oAPIControl, _propnames, _propvalues):
        setPeerProperties(oAPIControl, _propnames, _propvalues)

    def setPeerProperties(self, _xControl, propnames, propvalues):
        oPeerTask = PeerTask(_xControl, propnames, propvalues)
        self.m_aPeerTasks.append(oPeerTask)

    '''
    assigns an arbitrary property to a control as soon as the peer is created
    Note: The property 'ImageUrl' should better be assigned with
    'setImageurl(...)', to consider the High Contrast Mode
    @param _ocontrolmodel
    @param _spropname
    @param _propvalue
    '''

    def setControlProperty(self, _ocontrolmodel, _spropname, _propvalue):
        oControlTask = self.ControlTask(_ocontrolmodel, _spropname, _propvalue)
        self.aControlTasks.append(oControlTask)

    '''
    Assigns an image to the property 'ImageUrl' of a dialog control.
    The image id must be assigned in a resource file within the wizards
    project wizards project
    @param _ocontrolmodel
    @param _nResId
    @param _nhcResId
    '''

    def setImageUrl(self, _ocontrolmodel, _nResId, _nhcResId):
        oImageUrlTask = ImageUrlTask(_ocontrolmodel,_nResId, _nhcResId)
        self.aImageUrlTasks.append(oImageUrlTask)

    '''
    Assigns an image to the property 'ImageUrl' of a dialog control.
    The image ids that the Resource urls point to
    may be assigned in a Resource file outside the wizards project
    @param _ocontrolmodel
    @param _sResourceUrl
    @param _sHCResourceUrl
    '''

    def setImageUrl(self, _ocontrolmodel, _sResourceUrl, _sHCResourceUrl):
        oImageUrlTask = ImageUrlTask(
            _ocontrolmodel, _sResourceUrl, _sHCResourceUrl)
        self.aImageUrlTasks.append(oImageUrlTask)

    '''
    Assigns an image to the property 'ImageUrl' of a dialog control.
    The image id must be assigned in a resource file within the wizards
    project wizards project
    @param _ocontrolmodel
    @param _oResource
    @param _oHCResource
    '''

    def setImageUrl(self, _ocontrolmodel, _oResource, _oHCResource):
        oImageUrlTask = self.ImageUrlTask(
            _ocontrolmodel, _oResource, _oHCResource)
        self.aImageUrlTasks.append(oImageUrlTask)
