/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
