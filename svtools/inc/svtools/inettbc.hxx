/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVTOOLS_INETTBC_HXX
#define _SVTOOLS_INETTBC_HXX

// includes *****************************************************************

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

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
    String                          aPlaceHolder;
    SvtMatchContext_Impl*           pCtx;
    SvtURLBox_Impl*                 pImp;
    INetProtocol                    eSmartProtocol;
    sal_Bool                            bAutoCompleteMode   : 1;
    sal_Bool                            bOnlyDirectories    : 1;
    sal_Bool                            bModified           : 1;
    sal_Bool                            bTryAutoComplete    : 1;
    sal_Bool                            bCtrlClick          : 1;
    sal_Bool                            bHistoryDisabled    : 1;
    sal_Bool                            bNoSelection        : 1;
    sal_Bool                            bIsAutoCompleteEnabled : 1;

    SVT_DLLPRIVATE sal_Bool                         ProcessKey( const KeyCode& rCode );
    SVT_DLLPRIVATE void                         TryAutoComplete( sal_Bool bForce );
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
    void                            SetOnlyDirectories( sal_Bool bDir = sal_True );
    void                            SetNoURLSelection( sal_Bool bSet = sal_True );
    INetProtocol                    GetSmartProtocol() const { return eSmartProtocol; }
    void                            SetSmartProtocol( INetProtocol eProt );
    sal_Bool                            IsCtrlOpen()
                                        { return bCtrlClick; }
    String                          GetURL();
    void                            DisableHistory();

    void                            UpdatePickList( );

    static String                   ParseSmart( String aText, String aBaseURL, String aWorkDir );

    void                            SetFilter(const String& _sFilter);
    void                            SetUrlFilter( const IUrlFilter* _pFilter );
    const IUrlFilter*               GetUrlFilter( ) const;

    inline void                     EnableAutocompletion( sal_Bool _bEnable = sal_True )
                                        { bIsAutoCompleteEnabled = _bEnable; }
    void SetPlaceHolder( const String& sPlaceHolder ) { aPlaceHolder = sPlaceHolder; }
    String GetPlaceHolder() { return aPlaceHolder; }
    bool MatchesPlaceHolder( const String& sToMatch ) { return ( ( aPlaceHolder.Len() > 0 ) && ( aPlaceHolder == sToMatch ) ); }
};

#endif

