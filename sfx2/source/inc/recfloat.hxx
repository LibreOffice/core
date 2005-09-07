/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recfloat.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:09:50 $
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

#ifndef _RECFLOAT_HXX
#define _RECFLOAT_HXX

// includes *******************************************************************

#include "childwin.hxx"
#include "basedlgs.hxx"

class SfxRecordingFloatWrapper_Impl : public SfxChildWindow
{
    SfxBindings*        pBindings;
public:
                        SfxRecordingFloatWrapper_Impl( Window* pParent ,
                                                USHORT nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo );
                        ~SfxRecordingFloatWrapper_Impl();

                        SFX_DECL_CHILDWINDOW(SfxRecordingFloatWrapper_Impl);
    virtual sal_Bool    QueryClose();
};

class SfxRecordingFloat_Impl : public SfxFloatingWindow
{
    SfxChildWindow*         pWrapper;
    ToolBox                 aTbx;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XToolbarController > xStopRecTbxCtrl;
public:
                        SfxRecordingFloat_Impl( SfxBindings* pBindings ,
                            SfxChildWindow* pChildWin ,
                            Window* pParent );
    virtual             ~SfxRecordingFloat_Impl();
    virtual BOOL        Close();
    virtual void        FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual void        StateChanged( StateChangedType nStateChange );

    DECL_LINK( Select, ToolBox * );
};

#endif


