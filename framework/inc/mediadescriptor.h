/*************************************************************************
 *
 *  $RCSfile: mediadescriptor.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 14:16:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_MEDIADESCRIPTOR_H_
#define __FRAMEWORK_MEDIADESCRIPTOR_H_

//_______________________________________________
// namespace

namespace framework{

static const ::rtl::OUString MEDIADESCRIPTOR_ASTEMPLATE         = ::rtl::OUString::createFromAscii("AsTemplate"        );
static const ::rtl::OUString MEDIADESCRIPTOR_CHARACTERSET       = ::rtl::OUString::createFromAscii("CharacterSet"      );
static const ::rtl::OUString MEDIADESCRIPTOR_DEEPDETECTION      = ::rtl::OUString::createFromAscii("DeepDetection"     );
static const ::rtl::OUString MEDIADESCRIPTOR_DETECTSERVICE      = ::rtl::OUString::createFromAscii("DetectService"     );
static const ::rtl::OUString MEDIADESCRIPTOR_EXTENSION          = ::rtl::OUString::createFromAscii("Extension"         );
static const ::rtl::OUString MEDIADESCRIPTOR_FILENAME           = ::rtl::OUString::createFromAscii("FileName"          ); // deprecated!
static const ::rtl::OUString MEDIADESCRIPTOR_FILTERNAME         = ::rtl::OUString::createFromAscii("FilterName"        );
static const ::rtl::OUString MEDIADESCRIPTOR_FILTEROPTIONS      = ::rtl::OUString::createFromAscii("FilterOptions"     );
static const ::rtl::OUString MEDIADESCRIPTOR_FORMAT             = ::rtl::OUString::createFromAscii("Format"            );
static const ::rtl::OUString MEDIADESCRIPTOR_FRAMENAME          = ::rtl::OUString::createFromAscii("FrameName"         );
static const ::rtl::OUString MEDIADESCRIPTOR_HIDDEN             = ::rtl::OUString::createFromAscii("Hidden"            );
static const ::rtl::OUString MEDIADESCRIPTOR_INPUTSTREAM        = ::rtl::OUString::createFromAscii("InputStream"       );
static const ::rtl::OUString MEDIADESCRIPTOR_INTERACTIONHANDLER = ::rtl::OUString::createFromAscii("InteractionHandler");
static const ::rtl::OUString MEDIADESCRIPTOR_JUMPMARK           = ::rtl::OUString::createFromAscii("JumpMark"          );
static const ::rtl::OUString MEDIADESCRIPTOR_MACROEXECUTIONMODE = ::rtl::OUString::createFromAscii("MacroExecutionMode");
static const ::rtl::OUString MEDIADESCRIPTOR_MEDIATYPE          = ::rtl::OUString::createFromAscii("MediaType"         );
static const ::rtl::OUString MEDIADESCRIPTOR_MINIMIZED          = ::rtl::OUString::createFromAscii("Minimized"         );
static const ::rtl::OUString MEDIADESCRIPTOR_OPENNEWVIEW        = ::rtl::OUString::createFromAscii("OpenNewView"       );
static const ::rtl::OUString MEDIADESCRIPTOR_OUTPUTSTREAM       = ::rtl::OUString::createFromAscii("OutputStream"      );
static const ::rtl::OUString MEDIADESCRIPTOR_PATTERN            = ::rtl::OUString::createFromAscii("Pattern"           );
static const ::rtl::OUString MEDIADESCRIPTOR_POSSIZE            = ::rtl::OUString::createFromAscii("PosSize"           );
static const ::rtl::OUString MEDIADESCRIPTOR_POSTDATA           = ::rtl::OUString::createFromAscii("PostData"          );
static const ::rtl::OUString MEDIADESCRIPTOR_POSTSTRING         = ::rtl::OUString::createFromAscii("PostString"        );
static const ::rtl::OUString MEDIADESCRIPTOR_PREVIEW            = ::rtl::OUString::createFromAscii("Preview"           );
static const ::rtl::OUString MEDIADESCRIPTOR_READONLY           = ::rtl::OUString::createFromAscii("ReadOnly"          );
static const ::rtl::OUString MEDIADESCRIPTOR_REFERRER           = ::rtl::OUString::createFromAscii("Referer"           );
static const ::rtl::OUString MEDIADESCRIPTOR_SILENT             = ::rtl::OUString::createFromAscii("Silent"            );
static const ::rtl::OUString MEDIADESCRIPTOR_STATUSINDICATOR    = ::rtl::OUString::createFromAscii("StatusIndicator"   );
static const ::rtl::OUString MEDIADESCRIPTOR_TEMPLATENAME       = ::rtl::OUString::createFromAscii("TemplateName"      );
static const ::rtl::OUString MEDIADESCRIPTOR_TEMPLATEREGIONNAME = ::rtl::OUString::createFromAscii("TemplateRegionName");
static const ::rtl::OUString MEDIADESCRIPTOR_TYPENAME           = ::rtl::OUString::createFromAscii("TypeName"          );
static const ::rtl::OUString MEDIADESCRIPTOR_UPDATEDOCMODE      = ::rtl::OUString::createFromAscii("UpdateDocMode"     );
static const ::rtl::OUString MEDIADESCRIPTOR_URL                = ::rtl::OUString::createFromAscii("URL"               );
static const ::rtl::OUString MEDIADESCRIPTOR_VERSION            = ::rtl::OUString::createFromAscii("Version"           );
static const ::rtl::OUString MEDIADESCRIPTOR_VIEWID             = ::rtl::OUString::createFromAscii("ViewId"            );
static const ::rtl::OUString MEDIADESCRIPTOR_REPAIRPACKAGE      = ::rtl::OUString::createFromAscii("RepairPackage"     );
static const ::rtl::OUString MEDIADESCRIPTOR_DOCUMENTTITLE      = ::rtl::OUString::createFromAscii("DocumentTitle"     );
static const ::rtl::OUString MEDIADESCRIPTOR_MODEL              = ::rtl::OUString::createFromAscii("Model"             );

} // namespace framework

#endif // #ifndef __FRAMEWORK_MEDIADESCRIPTOR_H_
