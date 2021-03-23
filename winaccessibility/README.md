Windows Accessibility Bridge.

This code provides a bridge between our internal Accessibility
interfaces (implemented on all visible 'things' in the suite: eg.
windows, buttons, entry boxes etc.) - and the Windows MSAA /
IAccessible2 COM interfaces that are familiar to windows users and
Accessible Technologies (ATs) such as the NVDA screen reader.

The code breaks into three bits:

source/service/
	+ the UNO service providing the accessibility bridge.
	  It essentially listens to events from the LibreOffice
	  core and creates and synchronises COM peers for our
	  internal accessibility objects when events arrive.

source/UAccCom/
	+ COM implementations of the MSAA / IAccessible2 interfaces
	  to provide native peers for the accessibility code.

source/UAccCOMIDL/
	+ COM Interface Definition Language (IDL) for UAccCom.

Here is one way of visualising the code / control flow

VCL <-> UNO toolkit <-> UNO a11y <-> win a11y <-> COM / IAccessible2
vcl/ <-> toolkit/ <-> accessibility/ <-> winaccessibility/ <-> UAccCom/

Threading

It's possible that the UNO components are called from threads other
than the main thread, so they have to be synchronized. It would be nice
to put the component into a UNO apartment (and the COM components into STA)
but UNO would spawn a new thread for it so it's not possible.
The COM components also call into the same global AccObjectWinManager
as the UNO components do so both have to be synchronized in the same way.
So we use the SolarMutex for all synchronization since anything else
would be rather difficult to make work.  Unfortunately there is a
pre-existing problem in vcl with Win32 Window creation and destruction
on non-main threads where a synchronous SendMessage is used while
the SolarMutex is locked that can cause deadlocks if the main thread is
waiting on the SolarMutex itself at that time and thus not handing the
Win32 message; this is easy to trigger with JunitTests but hopefully
not by actual end users.

Debugging / playing with winaccessibility

If NVDA is running when soffice starts, IA2 should be automatically enabled
and work as expected. In order to use 'accprobe' to debug
it is necessary to override the check for whether an AT (like NVDA) is
running; to do that use:

SAL_FORCE_IACCESSIBLE2=1 soffice.exe -writer

Then you can use accprobe to introspect the accessibility hierarchy
remotely, checkout:

http://accessibility.linuxfoundation.org/a11yweb/util/accprobe/

But often it's more useful to look at NVDA's text output window...

