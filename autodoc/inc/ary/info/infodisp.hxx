/*************************************************************************
 *
 *  $RCSfile: infodisp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:11:54 $
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

#ifndef ARY_INFO_INFODISP_HXX
#define ARY_INFO_INFODISP_HXX

// USED SERVICES
    // BASE CLASSES
// #include <udm/display.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace info
{

class CodeInfo;
class StdTag;
class BaseTag;
class ExceptionTag;
class ImplementsTag;
class KeywordTag;
class ParameterTag;
class SeeTag;
class TemplateTag;
class LabelTag;
class SinceTag;
class DT_Text;
class DT_MaybeLink;
class DT_Whitespace;
class DT_Eol;
class DT_Xml;



/** Displaying an ary::info::CodeInfo.

    @descr
    This class is an interface, but the functions are defaulted,
    to do nothing. so a derived class needn't implement all of them.
*/
class DocuDisplay
{
  public:
    virtual             ~DocuDisplay() {}

    virtual void        Display_CodeInfo(
                            const CodeInfo &    i_rData );

    virtual void        Display_StdTag(
                            const StdTag &      i_rData );
    virtual void        Display_BaseTag(
                            const BaseTag &     i_rData );
    virtual void        Display_ExceptionTag(
                            const ExceptionTag &
                                                i_rData );
    virtual void        Display_ImplementsTag(
                            const ImplementsTag &
                                                i_rData );
    virtual void        Display_KeywordTag(
                            const KeywordTag &  i_rData );
    virtual void        Display_ParameterTag(
                            const ParameterTag &
                                                i_rData );
    virtual void        Display_SeeTag(
                            const SeeTag &      i_rData );
    virtual void        Display_TemplateTag(
                            const TemplateTag & i_rData );
    virtual void        Display_LabelTag(
                            const LabelTag &    i_rData );
    virtual void        Display_SinceTag(
                            const ary::info::SinceTag &
                                                i_rData );
    virtual void        Display_DT_Text(
                            const DT_Text &     i_rData );
    virtual void        Display_DT_MaybeLink(
                            const DT_MaybeLink& i_rData );
    virtual void        Display_DT_Whitespace(
                            const DT_Whitespace &
                                                i_rData );
    virtual void        Display_DT_Eol(
                            const DT_Eol &      i_rData );
    virtual void        Display_DT_Xml(
                            const ary::info::DT_Xml &
                                                i_rData );
};


}
}

#endif


