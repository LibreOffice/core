/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: infodisp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:16:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ARY_INFO_INFODISP_HXX
#define ARY_INFO_INFODISP_HXX
//  KORR_DEPRECATED_3.0

// BASE CLASSES
// USED SERVICES




namespace ary
{
namespace info
{
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



/** Displaying an ary::doc::OldCppDocu.

    @descr
    This class is an interface, but the functions are defaulted,
    to do nothing. so a derived class needn't implement all of them.
*/
class DocuDisplay
{
  public:
    virtual             ~DocuDisplay() {}

    virtual void        Display_StdTag(
                            const StdTag &      i_rData ) = 0;
    virtual void        Display_BaseTag(
                            const BaseTag &     i_rData ) = 0;
    virtual void        Display_ExceptionTag(
                            const ExceptionTag &
                                                i_rData ) = 0;
    virtual void        Display_ImplementsTag(
                            const ImplementsTag &
                                                i_rData ) = 0;
    virtual void        Display_KeywordTag(
                            const KeywordTag &  i_rData ) = 0;
    virtual void        Display_ParameterTag(
                            const ParameterTag &
                                                i_rData ) = 0;
    virtual void        Display_SeeTag(
                            const SeeTag &      i_rData ) = 0;
    virtual void        Display_TemplateTag(
                            const TemplateTag & i_rData ) = 0;
    virtual void        Display_LabelTag(
                            const LabelTag &    i_rData ) = 0;
    virtual void        Display_SinceTag(
                            const ary::info::SinceTag &
                                                i_rData ) = 0;
    virtual void        Display_DT_Text(
                            const DT_Text &     i_rData ) = 0;
    virtual void        Display_DT_MaybeLink(
                            const DT_MaybeLink& i_rData ) = 0;
    virtual void        Display_DT_Whitespace(
                            const DT_Whitespace &
                                                i_rData ) = 0;
    virtual void        Display_DT_Eol(
                            const DT_Eol &      i_rData ) = 0;
    virtual void        Display_DT_Xml(
                            const ary::info::DT_Xml &
                                                i_rData ) = 0;
};




}
}
#endif
