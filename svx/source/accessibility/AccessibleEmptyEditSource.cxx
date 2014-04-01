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




// Global header


#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>



// Project-local header



#include "AccessibleEmptyEditSource.hxx"
#include <svx/unoshtxt.hxx>

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
        virtual ~AccessibleProxyEditSource_Impl();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder() SAL_OVERRIDE;
        SvxViewForwarder*       GetViewForwarder() SAL_OVERRIDE;
        SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) SAL_OVERRIDE;

        SvxEditSource*          Clone() const SAL_OVERRIDE;

        void                    UpdateData() SAL_OVERRIDE;

        SfxBroadcaster&         GetBroadcaster() const SAL_OVERRIDE;

    private:
        SvxTextEditSource       maEditSource;

    };

    /** Dummy class, faking exactly one empty paragraph for EditEngine accessibility
     */
    class AccessibleEmptyEditSource_Impl : public SvxEditSource, public SvxViewForwarder, public SvxTextForwarder, public SfxBroadcaster
    {
    public:

        AccessibleEmptyEditSource_Impl() {}
        virtual ~AccessibleEmptyEditSource_Impl() {}

        // from the SfxListener interface
        void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

        // SvxEditSource
        SvxTextForwarder*       GetTextForwarder() SAL_OVERRIDE { return this; }
        SvxViewForwarder*       GetViewForwarder() SAL_OVERRIDE { return this; }
        SvxEditSource*          Clone() const SAL_OVERRIDE { return NULL; }
        void                    UpdateData() SAL_OVERRIDE {}
        SfxBroadcaster&         GetBroadcaster() const SAL_OVERRIDE { return *(const_cast<AccessibleEmptyEditSource_Impl*>(this)); }

        // SvxTextForwarder
        sal_Int32          GetParagraphCount() const SAL_OVERRIDE { return 1; }
        sal_Int32          GetTextLen( sal_Int32 /*nParagraph*/ ) const SAL_OVERRIDE { return 0; }
        OUString        GetText( const ESelection& /*rSel*/ ) const SAL_OVERRIDE { return OUString(); }
        SfxItemSet      GetAttribs( const ESelection& /*rSel*/, sal_Bool /*bOnlyHardAttrib*/ = 0 ) const SAL_OVERRIDE
        {
            // AW: Very dangerous: The former implementation used a SfxItemPool created on the
            // fly which of course was deleted again ASAP. Thus, the returned SfxItemSet was using
            // a deleted Pool by design.
            return SfxItemSet(SdrObject::GetGlobalDrawObjectItemPool());
        }
        SfxItemSet      GetParaAttribs( sal_Int32 /*nPara*/ ) const SAL_OVERRIDE { return GetAttribs(ESelection()); }
        void            SetParaAttribs( sal_Int32 /*nPara*/, const SfxItemSet& /*rSet*/ ) SAL_OVERRIDE {}
        void            RemoveAttribs( const ESelection& /*rSelection*/, bool /*bRemoveParaAttribs*/, sal_uInt16 /*nWhich*/ ) SAL_OVERRIDE {}
        void            GetPortions( sal_Int32 /*nPara*/, std::vector<sal_Int32>& /*rList*/ ) const SAL_OVERRIDE {}

        sal_uInt16          GetItemState( const ESelection& /*rSel*/, sal_uInt16 /*nWhich*/ ) const SAL_OVERRIDE { return 0; }
        sal_uInt16          GetItemState( sal_Int32 /*nPara*/, sal_uInt16 /*nWhich*/ ) const SAL_OVERRIDE { return 0; }

        SfxItemPool*    GetPool() const SAL_OVERRIDE { return NULL; }

        void            QuickInsertText( const OUString& /*rText*/, const ESelection& /*rSel*/ ) SAL_OVERRIDE {}
        void            QuickInsertField( const SvxFieldItem& /*rFld*/, const ESelection& /*rSel*/ ) SAL_OVERRIDE {}
        void            QuickSetAttribs( const SfxItemSet& /*rSet*/, const ESelection& /*rSel*/ ) SAL_OVERRIDE {}
        void            QuickInsertLineBreak( const ESelection& /*rSel*/ ) SAL_OVERRIDE {}

        const SfxItemSet * GetEmptyItemSetPtr() SAL_OVERRIDE { return 0; }

        void        AppendParagraph() SAL_OVERRIDE {}
        sal_Int32  AppendTextPortion( sal_Int32 /*nPara*/, const OUString & /*rText*/, const SfxItemSet & /*rSet*/ ) SAL_OVERRIDE { return 0; }

        //XTextCopy
        void        CopyText(const SvxTextForwarder& ) SAL_OVERRIDE {}

        OUString    CalcFieldValue( const SvxFieldItem& /*rField*/, sal_Int32 /*nPara*/, sal_Int32 /*nPos*/, Color*& /*rpTxtColor*/, Color*& /*rpFldColor*/ ) SAL_OVERRIDE
        {
            return  OUString();
        }
        void            FieldClicked( const SvxFieldItem&, sal_Int32, sal_Int32 ) SAL_OVERRIDE {;}

        bool            IsValid() const SAL_OVERRIDE { return true; }

        void            SetNotifyHdl( const Link& ) {}
        LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const SAL_OVERRIDE { return LANGUAGE_DONTKNOW; }
        sal_Int32       GetFieldCount( sal_Int32 ) const SAL_OVERRIDE { return 0; }
        EFieldInfo      GetFieldInfo( sal_Int32, sal_uInt16 ) const SAL_OVERRIDE { return EFieldInfo(); }
        EBulletInfo     GetBulletInfo( sal_Int32 ) const SAL_OVERRIDE { return EBulletInfo(); }
        Rectangle       GetCharBounds( sal_Int32, sal_Int32 ) const SAL_OVERRIDE { return Rectangle(); }
        Rectangle       GetParaBounds( sal_Int32 ) const SAL_OVERRIDE { return Rectangle(); }
        MapMode         GetMapMode() const SAL_OVERRIDE { return MapMode(); }
        OutputDevice*   GetRefDevice() const SAL_OVERRIDE { return NULL; }
        bool            GetIndexAtPoint( const Point&, sal_Int32&, sal_Int32& ) const SAL_OVERRIDE { return false; }
        bool            GetWordIndices( sal_Int32, sal_Int32, sal_Int32&, sal_Int32& ) const SAL_OVERRIDE { return false; }
        bool            GetAttributeRun( sal_Int32&, sal_Int32&, sal_Int32, sal_Int32, bool ) const SAL_OVERRIDE { return false; }
        sal_Int32       GetLineCount( sal_Int32 nPara ) const SAL_OVERRIDE { return nPara == 0 ? 1 : 0; }
        sal_Int32       GetLineLen( sal_Int32, sal_Int32 ) const SAL_OVERRIDE { return 0; }
        void            GetLineBoundaries( /*out*/sal_Int32 & rStart, /*out*/sal_Int32 & rEnd, sal_Int32 /*nParagraph*/, sal_Int32 /*nLine*/ ) const SAL_OVERRIDE  { rStart = rEnd = 0; }
        sal_Int32       GetLineNumberAtIndex( sal_Int32 /*nPara*/, sal_Int32 /*nIndex*/ ) const SAL_OVERRIDE   { return 0; }

        // the following two methods would, strictly speaking, require
        // a switch to a real EditSource, too. Fortunately, the
        // AccessibleEditableTextPara implementation currently always
        // calls GetEditViewForwarder(true) before doing
        // changes. Thus, we rely on this behabviour here (problem
        // when that changes: via accessibility API, it would no
        // longer be possible to enter text in previously empty
        // shapes).
        bool            Delete( const ESelection& ) SAL_OVERRIDE { return false; }
        bool            InsertText( const OUString&, const ESelection& ) SAL_OVERRIDE { return false; }
        bool            QuickFormatDoc( bool ) SAL_OVERRIDE { return true; }
        sal_Int16       GetDepth( sal_Int32 ) const SAL_OVERRIDE { return -1; }
        bool            SetDepth( sal_Int32, sal_Int16 ) SAL_OVERRIDE { return true; }

        Rectangle       GetVisArea() const SAL_OVERRIDE { return Rectangle(); }
        Point           LogicToPixel( const Point& rPoint, const MapMode& /*rMapMode*/ ) const SAL_OVERRIDE { return rPoint; }
        Point           PixelToLogic( const Point& rPoint, const MapMode& /*rMapMode*/ ) const SAL_OVERRIDE { return rPoint; }

    };


    // Implementing AccessibleProxyEditSource_Impl


    AccessibleProxyEditSource_Impl::AccessibleProxyEditSource_Impl( SdrObject&      rObj,
                                                                    SdrView&        rView,
                                                                    const Window&   rViewWindow ) :
        maEditSource( rObj, 0, rView, rViewWindow )
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

    SvxEditViewForwarder* AccessibleProxyEditSource_Impl::GetEditViewForwarder( bool bCreate )
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



    // Implementing AccessibleEmptyEditSource


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

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< SvxEditSource > pProxySource( new AccessibleProxyEditSource_Impl(mrObj, mrView, mrViewWindow) );
        ::std::auto_ptr< SvxEditSource > tmp = mpEditSource;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        mpEditSource = pProxySource;
        pProxySource = tmp;

        // register as listener
        StartListening( mpEditSource->GetBroadcaster() );

        // we've irrevocably a full EditSource now.
        mbEditSourceEmpty = false;
    }

    SvxEditViewForwarder* AccessibleEmptyEditSource::GetEditViewForwarder( bool bCreate )
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

    void AccessibleEmptyEditSource::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
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
        else if (pSdrHint && pSdrHint->GetObject()!=NULL)
        {
            // When the SdrObject just got a para outliner object then
            // switch the edit source.
            if (pSdrHint->GetObject()->GetOutlinerParaObject() != NULL)
                Switch2ProxyEditSource();
        }

        // forward messages
        Broadcast( rHint );
    }

} // end of namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
