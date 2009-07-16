Introduction
------------

This is the former location of the file: hid.lst
It is intended, that after a fresh checkout from svn you will not find it here anymore.
There will be no hid.lst anymore in source code repository for testautomation.

But the file hid.lst is essential for running automated tests with VCL
TestTool: Your OOo installation may already contain it at:
<OOo installation directory>/basis-link/program/hid.lst
(Depending on your installation you need to install the 'testtool' package)

As a fallback you will find the file hid.lst online at:
http://quaste.services.openoffice.org/index.php?option=com_quaste&task=tests_overview&workspace=DEV300&milestone=m51&download=2
(Please substitute DEV300 and m51 with the respective version numbers from your used OOo installation)
See announcement on QUASTe: http://quaste.services.openoffice.org/index.php?option=com_content&task=view&id=14&Itemid=1

As another fallback, the testautomation archives from Maho Nakata will already contain the hid.lst from QUASTe:
http://ooopackages.good-day.net/pub/OpenOffice.org/qa/testautomation/
Announcement is here: http://qa.openoffice.org/servlets/ReadMsg?list=dev&msgNo=12515

Action needed for tests with VCL TestTool? It depends;

Ideal processing
----------------

When checking out the VCL TestTool environment from svn;
OOo is already installed and the installation path is set
in the VCL TestTool seetings;
> http://wiki.services.openoffice.org/wiki/VCLTesttool

The first test run will recognize the absence of hid.lst in the directory
testautomation/global/hid, but finds hid.lst file in OOo
installation directory;
Then VCL TestTool will copy it from OOo installation to testautomation
directory - the test can start;

When using the testautomation archives from Nakata Maho,
> http://ooopackages.good-day.net/pub/OpenOffice.org/qa/testautomation/
there will still be already a hid.lst file in the testautomation
directory.

--------------------------------------------------------------------

Longer description to this change
---------------------------------

The change was necessarry since there were always several conflicts on
integration of several CWS's modifying the hid.lst file into one master.
The hid.lst is refused to checked in by Release Engineering after
building OOo, since they claim it is too late then and all files are
tagged then already in svn.
The file changes when new UI is put into, or removed from OOo;
It matches the number of an UI element known by OOo to a speaking name,
used in VCL TestTool test scripts.
There were always two hid.lst files in OOo:
- The first one, that is also the source for the second one, is
generated during building OOo;
- The second one, the one found in 'testautomation' was run through
several scripts to make it useable for testautomation and always
verified manually for errors;

With some fixes in DEV300m42/m45 and issue i101284 the generation of
hid.lst was changed during building of OOo, to use the resulting hid.lst
at the end also for testautomation;
This makes the second hid.lst now superfluous, since the hid.lst is also
delivered with every OOo installation;

The locations of the hid.lst:
- After an OOo build it is found at: solver/300/unxmacxp.pro/bin/hid.lst
  (due to a bug in deliver this one might be the wrong one, then you find
   it in: instsetoo_native/unxmacxi.pro/bin/hid.lst)
  To get the hid.lst build, you need to use 'configure' with the switch: --enable-hids
- In an installed OOo:
  Win32:  Program Files\Sun\StarOffice 9\Basis\program\hid.lst
  Mac OS: StarOffice.app/Contents/basis-link/program/hid.lst
  Linux: /opt/openoffice.org/basis3.2/program/hid.lst
- Online at http://quaste.services.openoffice.org/index.php?option=com_quaste&task=tests_overview&workspace=DEV300&milestone=m51&download=2
  With the respected values for the version replaced.

The file is finaly needed at: testautomation/global/hid/hid.lst

When using testautomation packages from:
> http://ooopackages.good-day.net/pub/OpenOffice.org/qa/testautomation/
No additional action should be required, since a hid.lst will be at the
needed location.

When checking out testautomation from svn, hid.lst is tried to find and 
copied by VCL TestTool on first test run.

When manual action is required
------------------------------

In case the target directory for hid.lst is not writeable by the user
who is running the test, manual action is required:
Open your VCL TestTool settings and change the hid directory path
to the basis program directory of your OOo installation; E.g.:
openoffice.org3/basis-link/program
> http://wiki.services.openoffice.org/wiki/VCLTesttool

There is also some output in the resultfile of the test, that should
give some hint, about the hid.lst; Either it was already there, got
copied from OOo or simply just not found.

If no hid.lst file is found, or could not be copied a messagebox comes up,
telling the reason and testing is aborted. Then please read the 
additional messages in the resultfile.

There might also be the case where the version of the hid.lst does not match the
version of the used OOo. Then there is an informal message shown.
The version should not differ, please check and correct the location of
the hid.lst file. Some reasons might be:
- the build was not done with configure switch  --enable-hids;
  then the hid.lst comes from helpcontent2/helpers/hid.lst, which
  is a constant one, which is far too old.
As for the VCL TestTool environment, the hid.lst file is also version dependant to OOo.

Current warnings in results
---------------------------

There are currenty about 99 warnings in the beginning, which are already 
fixed in CWS dba32e: i103219, butis not yet integrated into the master.

Getting help
------------

If nothing helps, do not hesitate to ask on maling list: 
dev@qa.openoffice.org
Start your subject with '[automation]: '

