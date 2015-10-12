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
                                        const vcl::Window&   rViewWindow );
        virtual ~AccessibleProxyEditSource_Impl();

        // from the SvxEditSource interface
        SvxTextForwarder*       GetTextForwarder() override;
        SvxViewForwarder*       GetViewForwarder() override;
        SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) override;

        SvxEditSource*          Clone() const override;

        void                    UpdateData() override;

        SfxBroadcaster&         GetBroadcaster() const override;

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

        // SvxEditSource
        SvxTextForwarder*       GetTextForwarder() override { return this; }
        SvxViewForwarder*       GetViewForwarder() override { return this; }
        SvxEditSource*          Clone() const override { return NULL; }
        void                    UpdateData() override {}
        SfxBroadcaster&         GetBroadcaster() const override { return *(const_cast<AccessibleEmptyEditSource_Impl*>(this)); }

        // SvxTextForwarder
        sal_Int32          GetParagraphCount() const override { return 1; }
        sal_Int32          GetTextLen( sal_Int32 /*nParagraph*/ ) const override { return 0; }
        OUString           GetText( const ESelection& /*rSel*/ ) const override { return OUString(); }
        SfxItemSet         GetAttribs( const ESelection& /*rSel*/, EditEngineAttribs /*nOnlyHardAttrib*/ = EditEngineAttribs_All ) const override
        {
            // AW: Very dangerous: The former implementation used a SfxItemPool created on the
            // fly which of course was deleted again ASAP. Thus, the returned SfxItemSet was using
            // a deleted Pool by design.
            return SfxItemSet(SdrObject::GetGlobalDrawObjectItemPool());
        }
        SfxItemSet      GetParaAttribs( sal_Int32 /*nPara*/ ) const override { return GetAttribs(ESelection()); }
        void            SetParaAttribs( sal_Int32 /*nPara*/, const SfxItemSet& /*rSet*/ ) override {}
        void            RemoveAttribs( const ESelection& /*rSelection*/, bool /*bRemoveParaAttribs*/, sal_uInt16 /*nWhich*/ ) override {}
        void            GetPortions( sal_Int32 /*nPara*/, std::vector<sal_Int32>& /*rList*/ ) const override {}

        SfxItemState    GetItemState( const ESelection& /*rSel*/, sal_uInt16 /*nWhich*/ ) const override { return SfxItemState::UNKNOWN; }
        SfxItemState    GetItemState( sal_Int32 /*nPara*/, sal_uInt16 /*nWhich*/ ) const override { return SfxItemState::UNKNOWN; }

        SfxItemPool*    GetPool() const override { return NULL; }

        void            QuickInsertText( const OUString& /*rText*/, const ESelection& /*rSel*/ ) override {}
        void            QuickInsertField( const SvxFieldItem& /*rFld*/, const ESelection& /*rSel*/ ) override {}
        void            QuickSetAttribs( const SfxItemSet& /*rSet*/, const ESelection& /*rSel*/ ) override {}
        void            QuickInsertLineBreak( const ESelection& /*rSel*/ ) override {}

        const SfxItemSet * GetEmptyItemSetPtr() override { return 0; }

        void        AppendParagraph() override {}
        sal_Int32  AppendTextPortion( sal_Int32 /*nPara*/, const OUString & /*rText*/, const SfxItemSet & /*rSet*/ ) override { return 0; }

        //XTextCopy
        void        CopyText(const SvxTextForwarder& ) override {}

        OUString    CalcFieldValue( const SvxFieldItem& /*rField*/, sal_Int32 /*nPara*/, sal_Int32 /*nPos*/, Color*& /*rpTxtColor*/, Color*& /*rpFldColor*/ ) override
        {
            return  OUString();
        }
        void            FieldClicked( const SvxFieldItem&, sal_Int32, sal_Int32 ) override {;}

        bool            IsValid() const override { return true; }

        LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const override { return LANGUAGE_DONTKNOW; }
        sal_Int32       GetFieldCount( sal_Int32 ) const override { return 0; }
        EFieldInfo      GetFieldInfo( sal_Int32, sal_uInt16 ) const override { return EFieldInfo(); }
        EBulletInfo     GetBulletInfo( sal_Int32 ) const override { return EBulletInfo(); }
        Rectangle       GetCharBounds( sal_Int32, sal_Int32 ) const override { return Rectangle(); }
        Rectangle       GetParaBounds( sal_Int32 ) const override { return Rectangle(); }
        MapMode         GetMapMode() const override { return MapMode(); }
        OutputDevice*   GetRefDevice() const override { return NULL; }
        bool            GetIndexAtPoint( const Point&, sal_Int32&, sal_Int32& ) const override { return false; }
        bool            GetWordIndices( sal_Int32, sal_Int32, sal_Int32&, sal_Int32& ) const override { return false; }
        bool            GetAttributeRun( sal_Int32&, sal_Int32&, sal_Int32, sal_Int32, bool ) const override { return false; }
        sal_Int32       GetLineCount( sal_Int32 nPara ) const override { return nPara == 0 ? 1 : 0; }
        sal_Int32       GetLineLen( sal_Int32, sal_Int32 ) const override { return 0; }
        void            GetLineBoundaries( /*out*/sal_Int32 & rStart, /*out*/sal_Int32 & rEnd, sal_Int32 /*nParagraph*/, sal_Int32 /*nLine*/ ) const override  { rStart = rEnd = 0; }
        sal_Int32       GetLineNumberAtIndex( sal_Int32 /*nPara*/, sal_Int32 /*nIndex*/ ) const override   { return 0; }

        // the following two methods would, strictly speaking, require
        // a switch to a real EditSource, too. Fortunately, the
        // AccessibleEditableTextPara implementation currently always
        // calls GetEditViewForwarder(true) before doing
        // changes. Thus, we rely on this behabviour here (problem
        // when that changes: via accessibility API, it would no
        // longer be possible to enter text in previously empty
        // shapes).
        bool            Delete( const ESelection& ) override { return false; }
        bool            InsertText( const OUString&, const ESelection& ) override { return false; }
        bool            QuickFormatDoc( bool ) override { return true; }
        sal_Int16       GetDepth( sal_Int32 ) const override { return -1; }
        bool            SetDepth( sal_Int32, sal_Int16 ) override { return true; }

        Rectangle       GetVisArea() const override { return Rectangle(); }
        Point           LogicToPixel( const Point& rPoint, const MapMode& /*rMapMode*/ ) const override { return rPoint; }
        Point           PixelToLogic( const Point& rPoint, const MapMode& /*rMapMode*/ ) const override { return rPoint; }

    };


    // Implementing AccessibleProxyEditSource_Impl


    AccessibleProxyEditSource_Impl::AccessibleProxyEditSource_Impl( SdrObject&      rObj,
                                                                    SdrView&        rView,
                                                                    const vcl::Window&   rViewWindow ) :
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
                                                          const vcl::Window& rViewWindow ) :
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

        ::std::unique_ptr< SvxEditSource > pProxySource( new AccessibleProxyEditSource_Impl(mrObj, mrView, mrViewWindow) );
        mpEditSource.swap(pProxySource);

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
        const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint );

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
