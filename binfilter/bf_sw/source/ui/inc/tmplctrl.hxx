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
#ifndef _TMPLCTRL_HXX
#define _TMPLCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _SFXSTBITEM_HXX //autogen
#include <bf_sfx2/stbitem.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
namespace binfilter {


// class BookmarkToolBoxControl -------------------------------------------

class SwTemplateControl : public SfxStatusBarControl
{
protected:
public:

    SFX_DECL_STATUSBAR_CONTROL();

    SwTemplateControl( USHORT nId, StatusBar& rStb, SfxBindings& rBind );
    ~SwTemplateControl();

private:
    String  sTemplate;
};


} //namespace binfilter
#endif

