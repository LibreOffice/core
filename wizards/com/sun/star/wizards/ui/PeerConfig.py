from event.CommonListener import WindowListenerProcAdapter

'''
@author rpiterman
To change the template for this generated type comment go to
Window>Preferences>Java>Code Generation>Code and Comments
'''

class PeerConfig(object):

    def __init__(self, _oUnoDialog):
        self.oUnoDialog = _oUnoDialog
        self.oUnoDialog.xUnoDialog.addWindowListener(
            WindowListenerProcAdapter(self.windowShown))
        self.m_aPeerTasks = []

    class PeerTask(object):

        def __init__(self, _xControl,propNames_,propValues_):
            self.propnames = propNames_
            self.propvalues = propValues_
            self.xControl = _xControl

    def windowShown(self):
        try:
            for i in self.m_aPeerTasks:
                xVclWindowPeer = i.xControl.Peer
                xVclWindowPeer.setProperty(i.propnames, i.propvalues)

        except Exception:
            traceback.print_exc()

    '''
    @param oAPIControl an API control that the interface
        XControl can be derived from
    @param _propnames
    @param _propvalues
    '''

    def setPeerProperties(self, _xControl, propnames, propvalues):
        oPeerTask = self.PeerTask(_xControl, propnames, propvalues)
        self.m_aPeerTasks.append(oPeerTask)
