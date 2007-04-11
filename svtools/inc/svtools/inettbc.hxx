/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inettbc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:23:33 $
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

#ifndef _SVTOOLS_INETTBC_HXX
#define _SVTOOLS_INETTBC_HXX

// includes *****************************************************************

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

class IUrlFilter;
class SvtMatchContext_Impl;
class SvtURLBox_Impl;
class SVT_DLLPUBLIC SvtURLBox : public ComboBox
{
friend class SvtMatchContext_Impl;
friend class SvtURLBox_Impl;
    Link                            aOpenHdl;
    String                          aBaseURL;
    SvtMatchContext_Impl*           pCtx;
    SvtURLBox_Impl*                 pImp;
    INetProtocol                    eSmartProtocol;
    BOOL                            bAutoCompleteMode   : 1;
    BOOL                            bOnlyDirectories    : 1;
    BOOL                            bModified           : 1;
    BOOL                            bTryAutoComplete    : 1;
    BOOL                            bCtrlClick          : 1;
    BOOL                            bHistoryDisabled    : 1;
    BOOL                            bNoSelection        : 1;
    BOOL                            bIsAutoCompleteEnabled : 1;

    SVT_DLLPRIVATE BOOL                         ProcessKey( const KeyCode& rCode );
    SVT_DLLPRIVATE void                         TryAutoComplete( BOOL bForce );
    SVT_DLLPRIVATE void                            UpdatePicklistForSmartProtocol_Impl();
    DECL_DLLPRIVATE_LINK(                      AutoCompleteHdl_Impl, void* );
    using Window::ImplInit;
    SVT_DLLPRIVATE void                            ImplInit();

protected:
    virtual long                    Notify( NotifyEvent& rNEvt );
    virtual void                    Select();
    virtual void                    Modify();
    virtual long                    PreNotify( NotifyEvent& rNEvt );

public:
                                    SvtURLBox( Window* pParent, INetProtocol eSmart = INET_PROT_NOT_VALID );
                                    SvtURLBox( Window* pParent, WinBits _nStyle, INetProtocol eSmart = INET_PROT_NOT_VALID );
                                    SvtURLBox( Window* pParent, const ResId& _rResId, INetProtocol eSmart = INET_PROT_NOT_VALID );
                                    ~SvtURLBox();

    void                            SetBaseURL( const String& rURL );
    const String&                   GetBaseURL() const { return aBaseURL; }
    void                            SetOpenHdl( const Link& rLink ) { aOpenHdl = rLink; }
    const Link&                     GetOpenHdl() const { return aOpenHdl; }
    void                            SetOnlyDirectories( BOOL bDir = TRUE );
    void                            SetNoURLSelection( BOOL bSet = TRUE );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    BOOL                            IsCtrlOpen()
                                        { return bCtrlClick; }
    String                          GetURL();
    void                            DisableHistory();

    void                            UpdatePickList( );

    static String                   ParseSmart( String aText, String aBaseURL, String aWorkDir );

    void                            SetFilter(const String& _sFilter);
    void                            SetUrlFilter( const IUrlFilter* _pFilter );
    const IUrlFilter*               GetUrlFilter( ) const;

    inline void                     EnableAutocompletion( BOOL _bEnable = TRUE )
                                        { bIsAutoCompleteEnabled = _bEnable; }
};

#endif

