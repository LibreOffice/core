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

#ifndef ARY_DISDOCUM_HXX
#define ARY_DISDOCUM_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace csi
{
namespace dsapi
{
class DT_TextToken;
class DT_MupType;
class DT_MupMember;
class DT_MupConst;
class DT_Style;
class DT_EOL;
class DT_StdAtTag;
class DT_SeeAlsoAtTag;
class DT_ParameterAtTag;
class DT_SinceAtTag;
}   // namespace dsapi
}   // namespace csi


namespace ary
{
namespace inf
{



class DocumentationDisplay
{
  public:

    virtual             ~DocumentationDisplay() { }

    virtual void        Display_TextToken(
                            const csi::dsapi::DT_TextToken &
                                                i_rToken ) = 0;
    virtual void        Display_White() = 0;
    virtual void        Display_MupType(
                            const csi::dsapi::DT_MupType &  i_rToken ) = 0;
    virtual void        Display_MupMember(
                            const csi::dsapi::DT_MupMember &
                                                i_rToken ) = 0;
    virtual void        Display_MupConst(
                            const csi::dsapi::DT_MupConst &
                                                i_rToken ) = 0;
    virtual void        Display_Style(
                            const csi::dsapi::DT_Style &    i_rToken ) = 0;
    virtual void        Display_EOL() = 0;
    virtual void        Display_StdAtTag(
                            const csi::dsapi::DT_StdAtTag &
                                                i_rToken ) = 0;
    virtual void        Display_SeeAlsoAtTag(
                            const csi::dsapi::DT_SeeAlsoAtTag &
                                                i_rToken ) = 0;
    virtual void        Display_ParameterAtTag(
                            const csi::dsapi::DT_ParameterAtTag &
                                                i_rToken ) = 0;
    virtual void        Display_SinceAtTag(
                            const csi::dsapi::DT_SinceAtTag &
                                                i_rToken ) = 0;
};


class DocuTag_Display : public DocumentationDisplay
{
  public:
    // Dummies, implemented in source\ary_i\kernel\ci_atag2.cxx
    virtual void        Display_TextToken(
                            const csi::dsapi::DT_TextToken &
                                                i_rToken );
    virtual void        Display_White();
    virtual void        Display_MupType(
                            const csi::dsapi::DT_MupType &  i_rToken );
    virtual void        Display_MupMember(
                            const csi::dsapi::DT_MupMember &
                                                i_rToken );
    virtual void        Display_MupConst(
                            const csi::dsapi::DT_MupConst &
                                                i_rToken );
    virtual void        Display_Style(
                            const csi::dsapi::DT_Style &    i_rToken );
    virtual void        Display_EOL();
};

class DocuText_Display : public DocumentationDisplay
{
  public:
    // Dummies, implemented in source\ary_i\kernel\ci_text2.cxx
    virtual void        Display_StdAtTag(
                            const csi::dsapi::DT_StdAtTag &
                                                i_rToken );
    virtual void        Display_SeeAlsoAtTag(
                            const csi::dsapi::DT_SeeAlsoAtTag &
                                                i_rToken );
    virtual void        Display_ParameterAtTag(
                            const csi::dsapi::DT_ParameterAtTag &
                                                i_rToken );
    virtual void        Display_SinceAtTag(
                            const csi::dsapi::DT_SinceAtTag &
                                                i_rToken );
};



}   // namespace inf
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
