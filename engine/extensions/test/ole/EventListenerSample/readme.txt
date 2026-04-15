EventListener
========================================

The folder EventListener contains an MSDEV project that builds a DLL that
contains the EventListener.EvtListener ActiveX component. The component
implements the XEventListener interface according to the rules of the
OleBridge. The component will be used from the HTML page events.htm.


VBEventListener:
========================================

Contains a Visual Basic project that builds an ActiveX component that implements
com.sun.star.lang.XEventListener. Its ProgId is
VBasicEventListener.VBEventListener. The DLL should also be checked in. VB needs
its TLB, so it uses the same CLSIDs on the next build. The component will create
a message box when its disposing function has been called.

The project also contains a client that builds a Project1.exe (in the same
folder) that creates VBEventListener and adds it to a StarOffice document.
