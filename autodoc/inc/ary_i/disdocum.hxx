/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
