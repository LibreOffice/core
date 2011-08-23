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
#ifndef _SVX_TBXFORM_HXX
#define _SVX_TBXFORM_HXX

#ifndef _SFXTBXCTRL_HXX //autogen
#include <bf_sfx2/tbxctrl.hxx>
#endif




class FixedText;
namespace binfilter {

//========================================================================


//========================================================================


//========================================================================


//========================================================================
class SvxFmTbxCtlConfig : public SfxToolBoxControl
{
private:
    UINT16 nLastSlot;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlConfig( USHORT nId, ToolBox& rTbx, SfxBindings& );
    ~SvxFmTbxCtlConfig() {}

    virtual void				StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
};

//========================================================================
class SvxFmTbxCtlAbsRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlAbsRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );


};

//========================================================================
class SvxFmTbxCtlRecText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
};

//========================================================================
class SvxFmTbxCtlRecFromText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecFromText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
};

//========================================================================
class SvxFmTbxCtlRecTotal : public SfxToolBoxControl
{
    FixedText* pFixedText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecTotal( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
};

/*
//========================================================================
class SvxFmTbxCtlFilterText : public SfxToolBoxControl
{
    FixedText* pFixedText;
    XubString aText;
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlFilterText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~SvxFmTbxCtlFilterText();

    virtual Window*	CreateItemWindow( Window* pParent );
    virtual void	StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};
*/

//========================================================================
class SvxFmTbxNextRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();											 
    SvxFmTbxNextRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );	
};

//========================================================================
class SvxFmTbxPrevRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();											 
    SvxFmTbxPrevRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );	
};					


}//end of namespace binfilter
#endif

