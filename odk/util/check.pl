#*************************************************************************
#
#   $RCSfile: check.pl,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2003-10-20 13:14:53 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

#
# check - a perl script to check some files and directories if they exists
# A first simple check if the SDK was built completely.
#
#use IO::File;

$return = 0;

$StartDir = "$ARGV[0]";
$OperatingSystem = "$ARGV[1]";
$ExePrefix = "$ARGV[2]";

print "Check for $OperatingSystem\n";

if (-d "$StartDir") {
    # check binaries
    print "check binaries: ";
    if (-d "$StartDir/$OperatingSystem/bin") {
    my @binarylist = ( "idlc","idlcpp","cppumaker","javamaker","rdbmaker","regcomp",
                "regcompare","regmerge","regview","autodoc", "uno", "xml2cmp" );

    foreach $i (@binarylist)
    {
        if (! -e "$StartDir/$OperatingSystem/bin/$i$ExePrefix") {
        $return++;
        print "-";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";

    # packaging files
    print "check packaging files: ";
    if (! -e "$StartDir/index.html") {
    print ":";
    $return++;
    }
    if (-d "$StartDir/docs") {
    my @filelist = ( "install.html","DevelopersGuide_intro.html","notsupported.html",
             "sdk_styles.css", "tools.html", "images/black_dot.gif",
             "images/logo.gif", "images/sdk_logo.gif", "images/nada.gif");

    foreach $i (@filelist)
    {
        if (! -e "$StartDir/docs/$i") {
        $return++;
        print "-";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";

    # check file format specification
    print "check xml format specifiaction: ";
    if (! -e "$StartDir/docs/common/spec/xml_format/xml_specification.pdf") {
    print "-";
    $return++;
    }
    print "\n";

    #check configure files
    print "check config files: ";
    if ($OperatingSystem eq "windows") {
    if (! -e "$StartDir/configureWindows.bat") {
        print "-";
        $return++;
    }
    if (! -e "$StartDir/setsdkenv_windows.bat") {
        print "-";
        $return++;
    }
    } else {
    if (! -e "$StartDir/configure") {
        print "-";
        $return++;
    }
    if (! -e "$StartDir/configure.pl") {
        print "-";
        $return++;
    }
    if (! -e "$StartDir/setsdkenv_unix.in") {
        print "-";
        $return++;
    }
    }
    print "\n";

    #check setting files
    print "check setting files: ";
    if (-d "$StartDir/settings") {
    if (! -e "$StartDir/settings/settings.mk") {
        print "-";
        $return++;
    }
    if (! -e "$StartDir/settings/std.mk") {
        print "-";
        $return++;
    }
    if (! -e "$StartDir/settings/stdtarget.mk") {
        print "-";
        $return++;
    }
    } else {
    $return++;
    }
    print "\n";

    #check cpp docu, it is only a first and simple check
    # improvement required
    print "check cpp docu: ";
    if (-d "$StartDir/docs/cpp/ref") {
    if (! -e "$StartDir/docs/cpp/ref/index.html") {
        print ":";
        $return++;
    }
    if (! -d "$StartDir/docs/cpp/ref/index-files") {
        print ":";
        $return++;
    }
    if (! -e "$StartDir/docs/cpp/ref/index-files/index-10.html") {
        print ":";
        $return++;
    }

    my @dir_list = ( "com","com/sun","com/sun/star","com/sun/star/uno","com/sun/star/uno/Any","com/sun/star/uno/Type","com/sun/star/uno/Array","com/sun/star/uno/WeakReferenceHelper","com/sun/star/uno/Reference","com/sun/star/uno/WeakReference","com/sun/star/uno/Environment","com/sun/star/uno/Sequence","com/sun/star/uno/BaseReference","com/sun/star/uno/Mapping","com/sun/star/uno/ContextLayer","com/sun/star/uno/TypeDescription","osl","osl/File","osl/Pipe","osl/FileStatus","osl/FileBase","osl/Guard","osl/Mutex","osl/VolumeInfo","osl/GetGlobalMutex","osl/Security","osl/Profile","osl/DatagramSocket","osl/SocketAddr","osl/StreamPipe","osl/ResettableGuard","osl/AcceptorSocket","osl/ClearableGuard","osl/VolumeDevice","rtl","rtl/Uri","rtl/math","rtl/OUStringHash","rtl/MalformedUriException","rtl/OUStringBuffer","rtl/OUString","rtl/Reference","rtl/ByteSequence","rtl/OLocale","rtl/Logfile","rtl/OString","rtl/IReference","rtl/OStringBuffer","rtl/OStringHash","_typelib_CompoundTypeDescription","cppu","cppu/ContextEntry_Init","cppu/ImplInheritanceHelper10","cppu/ImplInheritanceHelper11","cppu/ImplInheritanceHelper12","cppu/WeakAggImplHelper1","cppu/WeakAggImplHelper2","cppu/WeakAggImplHelper3","cppu/WeakAggImplHelper4","cppu/WeakAggImplHelper5","cppu/WeakAggImplHelper6","cppu/WeakAggImplHelper7","cppu/WeakAggImplHelper8","cppu/WeakAggImplHelper9","cppu/OMultiTypeInterfaceContainerHelperInt32","cppu/AccessControl","cppu/OPropertyArrayHelper","cppu/ImplHelper1","cppu/ImplHelper2","cppu/ImplHelper3","cppu/ImplHelper4","cppu/ImplHelper5","cppu/ImplHelper6","cppu/ImplHelper7","cppu/ImplHelper8","cppu/ImplHelper9","cppu/WeakComponentImplHelper10","cppu/WeakComponentImplHelper11","cppu/WeakComponentImplHelper12","cppu/UnoUrl","cppu/WeakComponentImplHelper1","cppu/WeakComponentImplHelper2","cppu/WeakComponentImplHelper3","cppu/WeakComponentImplHelper4","cppu/WeakComponentImplHelper5","cppu/WeakComponentImplHelper6","cppu/WeakComponentImplHelper7","cppu/WeakComponentImplHelper8","cppu/WeakComponentImplHelper9","cppu/OInterfaceIteratorHelper","cppu/OMultiTypeInterfaceContainerHelper","cppu/UnoUrlDescriptor","cppu/IPropertyArrayHelper","cppu/OBroadcastHelperVar","cppu/OComponentHelper","cppu/OWeakAggObject","cppu/ImplementationEntry","cppu/WeakImplHelper10","cppu/WeakImplHelper11","cppu/WeakImplHelper12","cppu/OPropertySetHelper","cppu/ImplHelper10","cppu/ImplHelper11","cppu/ImplHelper12","cppu/WeakAggImplHelper10","cppu/WeakAggImplHelper11","cppu/WeakAggImplHelper12","cppu/ImplInheritanceHelper1","cppu/ImplInheritanceHelper2","cppu/ImplInheritanceHelper3","cppu/ImplInheritanceHelper4","cppu/ImplInheritanceHelper5","cppu/ImplInheritanceHelper6","cppu/ImplInheritanceHelper7","cppu/ImplInheritanceHelper8","cppu/ImplInheritanceHelper9","cppu/OTypeCollection","cppu/WeakAggComponentImplHelper10","cppu/WeakAggComponentImplHelper11","cppu/WeakAggComponentImplHelper12","cppu/WeakAggComponentImplHelper1","cppu/WeakAggComponentImplHelper2","cppu/WeakAggComponentImplHelper3","cppu/WeakAggComponentImplHelper4","cppu/WeakAggComponentImplHelper5","cppu/WeakAggComponentImplHelper6","cppu/WeakAggComponentImplHelper7","cppu/WeakAggComponentImplHelper8","cppu/WeakAggComponentImplHelper9","cppu/OMultiTypeInterfaceContainerHelperVar","cppu/OInterfaceContainerHelper","cppu/OImplementationId","cppu/AggImplInheritanceHelper1","cppu/AggImplInheritanceHelper2","cppu/AggImplInheritanceHelper3","cppu/AggImplInheritanceHelper4","cppu/AggImplInheritanceHelper5","cppu/AggImplInheritanceHelper6","cppu/AggImplInheritanceHelper7","cppu/AggImplInheritanceHelper8","cppu/AggImplInheritanceHelper9","cppu/AggImplInheritanceHelper10","cppu/AggImplInheritanceHelper11","cppu/AggImplInheritanceHelper12","cppu/WeakImplHelper1","cppu/WeakImplHelper2","cppu/WeakImplHelper3","cppu/WeakImplHelper4","cppu/WeakImplHelper5","cppu/WeakImplHelper6","cppu/WeakImplHelper7","cppu/WeakImplHelper8","cppu/WeakImplHelper9","cppu/OWeakObject","__store_FindData","_rtl_StandardModuleCount","RTUik","RTConstValue","_typelib_TypeDescriptionReference","_typelib_InterfaceMethodTypeDescription","store","RegistryKey","_typelib_Union_Init","_sal_Sequence","_typelib_Parameter_Init","_typelib_TypeDescription","_uno_Environment","_typelib_InterfaceAttributeTypeDescription","uno_Context","RegistryTypeReaderLoader","_rtl_ModuleCount","_uno_ExtEnvironment","_typelib_IndirectTypeDescription","remote_DisposingListener","remote_Interface","Registry_Api","_oslFileStatus","_typelib_InterfaceMemberTypeDescription","RegistryValueList","RegistryTypeWriter_Api","_rtl_TextEncodingInfo","namespace_anonymous_1","remote_Connection","RegistryLoader","_oslVolumeInfo","_uno_Interface","RegistryTypeWriterLoader","_typelib_InterfaceTypeDescription","_uno_Mapping","Registry","RegistryTypeReader_Api","remote_Context","_typelib_Uik","remote_InstanceProvider","_typelib_ArrayTypeDescription","RegistryKeyArray","RegistryTypeReader","RegistryKeyNames","RTConstValueUnion","_typelib_UnionTypeDescription","_uno_Any","RegistryTypeWriter","_rtl_Locale","_typelib_CompoundMember_Init","_typelib_EnumTypeDescription","_typelib_MethodParameter");

    foreach $i (@dir_list)
    {
        if (! -d "$StartDir/docs/cpp/ref/names/$i") {
        $return++;
        print "-";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";

    #check java docu, it is only a first and simple check
    # improvement required
    print "check java docu: ";
    if (-d "$StartDir/docs/java/ref") {
    if (! -e "$StartDir/docs/java/ref/index.html") {
        print ":";
        $return++;
    }
    if (! -d "$StartDir/docs/cpp/ref/index-files") {
        print ":";
        $return++;
    }
    if (! -e "$StartDir/docs/cpp/ref/index-files/index-10.html") {
        print ":";
        $return++;
    }

    my @dir_list = ( "lib","lib/uno","lib/uno/helper","lib/uno/helper/class-use","uno",
             "uno/class-use","comp","comp/helper","comp/helper/class-use","tools",
             "tools/uno","tools/uno/class-use");

    foreach $i (@dir_list)
    {
        if (! -d "$StartDir/docs/java/ref/com/sun/star/$i") {
        $return++;
        print "-";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";

    #check examples, it is only a first and simple check
    # improvement required
    print "check examples: ";
    if (-d "$StartDir/examples") {
    if (! -e "$StartDir/examples/examples.html") {
        print ":";
        $return++;
    }
    if (! -e "$StartDir/examples/DevelopersGuide/examples.html") {
        print ":";
        $return++;
    }

    my @dirlist = ( "OLE","OLE/vbscript","OLE/delphi","OLE/delphi/InsertTables","OLE/activex","cpp","cpp/counter","cpp/remoteclient","cpp/DocumentLoader","java","java/Text","java/ToDo","java/NotesAccess","java/ConverterServlet","java/Inspector","java/Drawing","java/MinimalComponent","java/PropertySet","java/Spreadsheet","java/DocumentHandling","java/DocumentHandling/test","basic","basic/text","basic/text/modifying_text_automatically","basic/text/creating_an_index","basic/sheet","basic/drawing","basic/forms_and_controls","basic/stock_quotes_updater","DevelopersGuide","DevelopersGuide/UCB","DevelopersGuide/UCB/data","DevelopersGuide/Text","DevelopersGuide/Charts","DevelopersGuide/Config","DevelopersGuide/Forms","DevelopersGuide/FirstSteps","DevelopersGuide/OfficeDev","DevelopersGuide/OfficeDev/FilterDevelopment","DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter","DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_cpp","DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_java","DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection","DevelopersGuide/OfficeDev/Linguistic","DevelopersGuide/OfficeDev/Clipboard","DevelopersGuide/OfficeDev/PathSettings","DevelopersGuide/OfficeDev/TerminationTest","DevelopersGuide/OfficeDev/DesktopEnvironment","DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib","DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/unix","DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/windows","DevelopersGuide/OfficeDev/DisableCommands","DevelopersGuide/OfficeDev/PathSubstitution","DevelopersGuide/Components","DevelopersGuide/Components/JavaComponent","DevelopersGuide/Components/Thumbs","DevelopersGuide/Components/Thumbs/org","DevelopersGuide/Components/Thumbs/org/openoffice","DevelopersGuide/Components/Thumbs/org/openoffice/comp","DevelopersGuide/Components/Thumbs/org/openoffice/comp/test","DevelopersGuide/Components/Thumbs/org/openoffice/test","DevelopersGuide/Components/CppComponent","DevelopersGuide/Components/Addons","DevelopersGuide/Components/Addons/JobsAddon","DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java","DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp","DevelopersGuide/BasicAndDialogs","DevelopersGuide/BasicAndDialogs/ToolkitControls","DevelopersGuide/BasicAndDialogs/CreatingDialogs","DevelopersGuide/ProfUNO","DevelopersGuide/ProfUNO/CppBinding","DevelopersGuide/ProfUNO/InterprocessConn","DevelopersGuide/ProfUNO/Lifetime","DevelopersGuide/Drawing","DevelopersGuide/Accessibility","DevelopersGuide/OfficeBean","DevelopersGuide/OfficeBean/OfficeWriterBean","DevelopersGuide/OfficeBean/SimpleBean","DevelopersGuide/Database","DevelopersGuide/Database/DriverSkeleton","DevelopersGuide/Spreadsheet");
    foreach $i (@dirlist)
    {
        if (! -d "$StartDir/examples/$i") {
        $return++;
        print "-";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";
} else {
    $return++;
}

if( $return != 0 )
{
    print "ERROR\n";
    unlink "$ARGV[3]";
} else {
    print "OK\n";
}
exit $return;
