The program tests the OleObjectFactory service which enables to use COM components
through XInvocation.

Requirements:

applicat.rdb has to be next to the executable. 
COM component: XCallback_Impl.Simple ( extensions/test/ole/unotocomcalls/XCallback_Impl )
ActiveX controls: AxTestComponent.Basic ( extensions/test/ole/AxTestComponents)
		  MFCCONTROL.MfcControlCtrl.1 (extensions/test/ole/MfcControl)

The project cannot be build in our environment therefore it must be an MSDEV project.
Reason: ATL uses min and max macros defined in windef.h. Our stl header undef min and max.
To generate the uno header use the make file. Then Set the include path directories to
the inc directory in the build tree.

