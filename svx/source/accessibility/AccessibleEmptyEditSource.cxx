/*************************************************************************
 *
 *  $RCSfile: AccessibleEmptyEditSource.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:46:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _MyEDITDATA_HXX
#include "editdata.hxx"
#endif

#ifndef _OUTLINER_HXX
#include "outliner.hxx"
#endif

#ifndef _SVDMODEL_HXX
#include "svdmodel.hxx"
#endif

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include "AccessibleEmptyEditSource.hxx"
#include "unoshtxt.hxx"

namespace accessibility
{

    /** This class simply wraps a SvxTextEditSource, forwarding all
        methods except the GetBroadcaster() call
     */
    class AccessibleProxyEditSource_Impl : public SvxEditSource
    {
    public:
        /** Construct AccessibleEmptyEditSource_Impl

            @param rBrdCast

            Proxy broadcaster to allow seamless flipping of edit source implementations. ProxyEditSource and EmptyEditSource
         */
        AccessibleProxyEditSource_Impl( SdrObject&      rObj,
                                        SdrView&        rView,
                                        const Window&   rViewWindow );
        ~AccessibleProxyEditSource_Impl();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder();
        SvxViewForwarder*       GetViewForwarder();
        SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );

        SvxEditSource*          Clone() const;

        void                    UpdateData();

        SfxBroadcaster&         GetBroadcaster() const;

    private:
        SvxTextEditSource       maEditSource;

    };

    /** Dummy class, faking exactly one empty paragraph for EditEngine accessibility
     */
    class AccessibleEmptyEditSource_Impl : public SvxEditSource, public SvxViewForwarder, public SvxTextForwarder, public SfxBroadcaster
    {
    public:

        AccessibleEmptyEditSource_Impl() {}
        ~AccessibleEmptyEditSource_Impl() {}

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

        // SvxEditSource
        SvxTextForwarder*       GetTextForwarder() { return this; }
        SvxViewForwarder*       GetViewForwarder() { return this; }
        SvxEditSource*          Clone() const { return NULL; }
        void                    UpdateData() {}
        SfxBroadcaster&         GetBroadcaster() const { return *(const_cast<AccessibleEmptyEditSource_Impl*>(this)); }

        // SvxTextForwarder
        USHORT          GetParagraphCount() const { return 1; }
        USHORT          GetTextLen( USHORT nParagraph ) const { return 0; }
        String          GetText( const ESelection& rSel ) const { return String(); }
        SfxItemSet      GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib = 0 ) const
        {
            String aDummyStr(RTL_CONSTASCII_USTRINGPARAM("Dummy"));
            SfxItemPool aPool(aDummyStr,0,0,NULL);
            return SfxItemSet(aPool);
        }
        SfxItemSet      GetParaAttribs( USHORT nPara ) const { return GetAttribs(ESelection()); }
        void            SetParaAttribs( USHORT nPara, const SfxItemSet& rSet ) {}
        void            GetPortions( USHORT nPara, SvUShorts& rList ) const {}

        USHORT          GetItemState( const ESelection& rSel, USHORT nWhich ) const { return 0; }
        USHORT          GetItemState( USHORT nPara, USHORT nWhich ) const { return 0; }

        SfxItemPool*    GetPool() const { return NULL; }

        void            QuickInsertText( const String& rText, const ESelection& rSel ) {}
        void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) {}
        void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) {}
        void            QuickInsertLineBreak( const ESelection& rSel ) {}

        XubString       CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
        {
            return  XubString();
        }
        BOOL            IsValid() const { return sal_True; }

        void            SetNotifyHdl( const Link& ) {}
        LanguageType    GetLanguage( USHORT, USHORT ) const { return LANGUAGE_DONTKNOW; }
        USHORT          GetFieldCount( USHORT nPara ) const { return 0; }
        EFieldInfo      GetFieldInfo( USHORT nPara, USHORT nField ) const { return EFieldInfo(); }
        EBulletInfo     GetBulletInfo( USHORT nPara ) const { return EBulletInfo(); }
        Rectangle       GetCharBounds( USHORT nPara, USHORT nIndex ) const { return Rectangle(); }
        Rectangle       GetParaBounds( USHORT nPara ) const { return Rectangle(); }
        MapMode         GetMapMode() const { return MapMode(); }
        OutputDevice*   GetRefDevice() const { return NULL; }
        sal_Bool        GetIndexAtPoint( const Point&, USHORT& nPara, USHORT& nIndex ) const { return sal_False; }
        sal_Bool        GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const { return sal_False; }
        sal_Bool        GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const { return sal_False; }
        USHORT          GetLineCount( USHORT nPara ) const { return nPara == 0 ? 1 : 0; }
        USHORT          GetLineLen( USHORT nPara, USHORT nLine ) const { return 0; }

        // the following two methods would, strictly speaking, require
        // a switch to a real EditSource, too. Fortunately, the
        // AccessibleEditableTextPara implementation currently always
        // calls GetEditViewForwarder(true) before doing
        // changes. Thus, we rely on this behabviour here (problem
        // when that changes: via accessibility API, it would no
        // longer be possible to enter text in previously empty
        // shapes).
        sal_Bool        Delete( const ESelection& ) { return sal_False; }
        sal_Bool        InsertText( const String&, const ESelection& ) { return sal_False; }
        sal_Bool        QuickFormatDoc( BOOL bFull=FALSE ) { return sal_True; }
        USHORT          GetDepth( USHORT nPara ) const { return 0; }
        sal_Bool        SetDepth( USHORT nPara, USHORT nNewDepth ) { return 0; }

        Rectangle       GetVisArea() const { return Rectangle(); }
        Point           LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const { return rPoint; }
        Point           PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const { return rPoint; }

    };

    // -------------------------------------------------------------------------
    // Implementing AccessibleProxyEditSource_Impl
    // -------------------------------------------------------------------------

    AccessibleProxyEditSource_Impl::AccessibleProxyEditSource_Impl( SdrObject&      rObj,
                                                                    SdrView&        rView,
                                                                    const Window&   rViewWindow ) :
        maEditSource( rObj, rView, rViewWindow )
    {
    }

    AccessibleProxyEditSource_Impl::~AccessibleProxyEditSource_Impl()
    {
    }

    SvxTextForwarder* AccessibleProxyEditSource_Impl::GetTextForwarder()
    {
        return maEditSource.GetTextForwarder();
    }

    SvxViewForwarder* AccessibleProxyEditSource_Impl::GetViewForwarder()
    {
        return maEditSource.GetViewForwarder();
    }

    SvxEditViewForwarder* AccessibleProxyEditSource_Impl::GetEditViewForwarder( sal_Bool bCreate )
    {
        return maEditSource.GetEditViewForwarder( bCreate );
    }

    SvxEditSource* AccessibleProxyEditSource_Impl::Clone() const
    {
        return maEditSource.Clone();
    }

    void AccessibleProxyEditSource_Impl::UpdateData()
    {
        maEditSource.UpdateData();
    }

    SfxBroadcaster& AccessibleProxyEditSource_Impl::GetBroadcaster() const
    {
        return maEditSource.GetBroadcaster();
    }


    // -------------------------------------------------------------------------
    // Implementing AccessibleEmptyEditSource
    // -------------------------------------------------------------------------

    AccessibleEmptyEditSource::AccessibleEmptyEditSource( SdrObject&    rObj,
                                                          SdrView&      rView,
                                                          const Window& rViewWindow ) :
        mpEditSource( new AccessibleEmptyEditSource_Impl() ),
        mrObj(rObj),
        mrView(rView),
        mrViewWindow(rViewWindow),
        mbEditSourceEmpty( true )
    {
        if( mrObj.GetModel() )
            StartListening( *mrObj.GetModel() );
    }

    AccessibleEmptyEditSource::~AccessibleEmptyEditSource()
    {
        if( !mbEditSourceEmpty )
        {
            // deregister as listener
            if( mpEditSource.get() )
                EndListening( mpEditSource->GetBroadcaster() );
        }
        else
        {
            if( mrObj.GetModel() )
                EndListening( *mrObj.GetModel() );
        }
    }

    SvxTextForwarder* AccessibleEmptyEditSource::GetTextForwarder()
    {
        if( !mpEditSource.get() )
            return NULL;

        return mpEditSource->GetTextForwarder();
    }

    SvxViewForwarder* AccessibleEmptyEditSource::GetViewForwarder()
    {
        if( !mpEditSource.get() )
            return NULL;

        return mpEditSource->GetViewForwarder();
    }

    void AccessibleEmptyEditSource::Switch2ProxyEditSource()
    {
        // deregister EmptyEditSource model listener
        if( mrObj.GetModel() )
            EndListening( *mrObj.GetModel() );

        ::std::auto_ptr< SvxEditSource > pProxySource( new AccessibleProxyEditSource_Impl(mrObj, mrView, mrViewWindow) );
        ::std::swap(pProxySource, mpEditSource);

        // register as listener
        StartListening( mpEditSource->GetBroadcaster() );

        // we've irrevocably a full EditSource now.
        mbEditSourceEmpty = false;
    }

    SvxEditViewForwarder* AccessibleEmptyEditSource::GetEditViewForwarder( sal_Bool bCreate )
    {
        if( !mpEditSource.get() )
            return NULL;

        // switch edit source, if not yet done
        if( mbEditSourceEmpty && bCreate )
            Switch2ProxyEditSource();

        return mpEditSource->GetEditViewForwarder( bCreate );
    }

    SvxEditSource* AccessibleEmptyEditSource::Clone() const
    {
        if( !mpEditSource.get() )
            return NULL;

        return mpEditSource->Clone();
    }

    void AccessibleEmptyEditSource::UpdateData()
    {
        if( mpEditSource.get() )
            mpEditSource->UpdateData();
    }

    SfxBroadcaster& AccessibleEmptyEditSource::GetBroadcaster() const
    {
        return *(const_cast<AccessibleEmptyEditSource*>(this));
    }

    void AccessibleEmptyEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

        if( pSdrHint && pSdrHint->GetKind() == HINT_BEGEDIT &&
            &mrObj == pSdrHint->GetObject() && mpEditSource.get() )
        {
            // switch edit source, if not yet done. This is necessary
            // to become a full-fledged EditSource the first time a
            // user start entering text in a previously empty object.
            if( mbEditSourceEmpty )
                Switch2ProxyEditSource();
        }

        // forward messages
        Broadcast( rHint );
    }

} // end of namespace accessibility

//------------------------------------------------------------------------
