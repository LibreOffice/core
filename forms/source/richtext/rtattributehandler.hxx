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

#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_RTATTRIBUTEHANDLER_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_RTATTRIBUTEHANDLER_HXX

#include "rtattributes.hxx"
#include <rtl/ref.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/frmdir.hxx>

class SfxItemSet;
class SfxPoolItem;
class SfxItemPool;

namespace frm
{



    //= ReferenceBase

    class ReferenceBase : public ::rtl::IReference
    {
    protected:
        oslInterlockedCount m_refCount;

    public:
        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        virtual ~ReferenceBase();
    };


    //= IAttributeHandler

    class IAttributeHandler : public ::rtl::IReference
    {
    public:
        virtual     AttributeId     getAttributeId( ) const = 0;
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const = 0;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const = 0;

    protected:
        ~IAttributeHandler() {}
    };


    //= AttributeHandler

    class AttributeHandler  :public ReferenceBase
                            ,public IAttributeHandler
    {
    private:
        AttributeId     m_nAttribute;
        WhichId         m_nWhich;

    protected:
        AttributeId getAttribute() const { return m_nAttribute; }
        WhichId     getWhich() const     { return m_nWhich;     }

    public:
        AttributeHandler( AttributeId _nAttributeId, WhichId _nWhichId );

        // IAttributeHandler
        virtual     AttributeId     getAttributeId( ) const;
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const = 0;

    protected:
        /// helper method calling implGetCheckState
        AttributeCheckState getCheckState( const SfxItemSet& _rAttribs ) const;

        /// helper method putting an item into a set, respecting a script type
        void        putItemForScript( SfxItemSet& _rAttribs, const SfxPoolItem& _rItem, ScriptType _nForScriptType ) const;

        // pseudo-abstract
        virtual     AttributeCheckState  implGetCheckState( const SfxPoolItem& _rItem ) const;

        // disambiguate IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        virtual ~AttributeHandler();
    };


    //= AttributeHandlerFactory

    class AttributeHandlerFactory
    {
    public:
        static ::rtl::Reference< IAttributeHandler > getHandlerFor( AttributeId _nAttributeId, const SfxItemPool& _rEditEnginePool );

    private:
        AttributeHandlerFactory();                                              // never implemented
        AttributeHandlerFactory( const AttributeHandlerFactory& );              // never implemented
        AttributeHandlerFactory& operator=( const AttributeHandlerFactory& );   // never implemented
        ~AttributeHandlerFactory();                                             // never implemented
    };


    //= ParaAlignmentHandler

    class ParaAlignmentHandler : public AttributeHandler
    {
    private:
        SvxAdjust   m_eAdjust;

    public:
        ParaAlignmentHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


    //= LineSpacingHandler

    class LineSpacingHandler : public AttributeHandler
    {
    private:
        sal_uInt16  m_nLineSpace;

    public:
        LineSpacingHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


    //= EscapementHandler

    class EscapementHandler : public AttributeHandler
    {
    private:
        SvxEscapement   m_eEscapement;

    public:
        EscapementHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


    //= SlotHandler

    class SlotHandler : public AttributeHandler
    {
    private:
        bool    m_bScriptDependent;

    public:
        SlotHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


    //= BooleanHandler

    class BooleanHandler : public AttributeHandler
    {
    public:
        BooleanHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


    //= FontSizeHandler

    class FontSizeHandler : public AttributeHandler
    {
    public:
        FontSizeHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


    //= ParagraphDirectionHandler

    class ParagraphDirectionHandler : public AttributeHandler
    {
    private:
        SvxFrameDirection   m_eParagraphDirection;
        SvxAdjust           m_eDefaultAdjustment;
        SvxAdjust           m_eOppositeDefaultAdjustment;

    public:
        ParagraphDirectionHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, ScriptType _nForScriptType ) const;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_RTATTRIBUTEHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
