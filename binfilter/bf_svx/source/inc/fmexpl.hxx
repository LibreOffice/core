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
#ifndef _SVX_FMEXPL_HXX
#define _SVX_FMEXPL_HXX







#ifndef _SFXDOCKWIN_HXX //autogen
#include <bf_sfx2/dockwin.hxx>
#endif












#ifndef _SVX_FMVIEW_HXX
#include "fmview.hxx"
#endif


class SdrObjListIter;
namespace binfilter {

class FmFormShell;
class SdrObject;
class FmFormModel;

//========================================================================

//========================================================================

//========================================================================

//========================================================================

//========================================================================

//========================================================================
class FmNavViewMarksChanged : public SfxHint
{
    FmFormView* pView;
public:
    FmNavViewMarksChanged(FmFormView* pWhichView) { pView = pWhichView; }
    virtual ~FmNavViewMarksChanged() {}

};

//========================================================================

//========================================================================


//========================================================================
// FmNavRequestSelectHint - jemand teilt dem NavigatorTree mit, dass er bestimmte Eintraege selektieren soll



//========================================================================


//========================================================================


//========================================================================
//............................................................................
namespace svxform
{
//............................................................................

    //========================================================================
    // class OFormComponentObserver
    //========================================================================

    //========================================================================
    //= NavigatorTreeModel
    //========================================================================

    //========================================================================


    //========================================================================

    //========================================================================
    class NavigatorFrameManager : public SfxChildWindow
    {
    public:
        NavigatorFrameManager( Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                          SfxChildWinInfo *pInfo );
        SFX_DECL_CHILDWINDOW( NavigatorFrameManager );
    };

//............................................................................
}	// namespace svxform
//............................................................................

}//end of namespace binfilter
#endif // _SVX_FMEXPL_HXX

