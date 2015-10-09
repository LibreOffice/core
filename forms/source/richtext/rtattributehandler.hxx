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
#include <svl/languageoptions.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/frmdir.hxx>
#include <salhelper/simplereferenceobject.hxx>

class SfxItemSet;
class SfxPoolItem;
class SfxItemPool;

namespace frm
{

    class AttributeHandler  : public salhelper::SimpleReferenceObject
    {
    private:
        AttributeId     m_nAttribute;
        WhichId         m_nWhich;

    protected:
        AttributeId getAttribute() const { return m_nAttribute; }
        WhichId     getWhich() const     { return m_nWhich;     }

    public:
        AttributeHandler( AttributeId _nAttributeId, WhichId _nWhichId );

        AttributeId             getAttributeId( ) const;
        virtual AttributeState  getState( const SfxItemSet& _rAttribs ) const;
        virtual void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const = 0;

    protected:
        /// helper method calling implGetCheckState
        AttributeCheckState     getCheckState( const SfxItemSet& _rAttribs ) const;

        /// helper method putting an item into a set, respecting a script type
        void                    putItemForScript( SfxItemSet& _rAttribs, const SfxPoolItem& _rItem, SvtScriptType _nForScriptType ) const;

        // pseudo-abstract
        virtual AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const;

    protected:
        virtual ~AttributeHandler();
    };

    namespace AttributeHandlerFactory
    {
        ::rtl::Reference< AttributeHandler > getHandlerFor( AttributeId _nAttributeId, const SfxItemPool& _rEditEnginePool );
    }

    class ParaAlignmentHandler : public AttributeHandler
    {
    private:
        SvxAdjust   m_eAdjust;

    public:
        explicit ParaAlignmentHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const SAL_OVERRIDE;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };

    class LineSpacingHandler : public AttributeHandler
    {
    private:
        sal_uInt16  m_nLineSpace;

    public:
        explicit LineSpacingHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const SAL_OVERRIDE;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };

    class EscapementHandler : public AttributeHandler
    {
    private:
        SvxEscapement   m_eEscapement;

    public:
        explicit EscapementHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const SAL_OVERRIDE;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };

    class SlotHandler : public AttributeHandler
    {
    private:
        bool    m_bScriptDependent;

    public:
        SlotHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const SAL_OVERRIDE;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };

    class BooleanHandler : public AttributeHandler
    {
    public:
        BooleanHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const SAL_OVERRIDE;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };

    class FontSizeHandler : public AttributeHandler
    {
    public:
        FontSizeHandler( AttributeId _nAttributeId, WhichId _nWhichId );

    public:
        virtual     AttributeState  getState( const SfxItemSet& _rAttribs ) const SAL_OVERRIDE;
        virtual     void            executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };

    class ParagraphDirectionHandler : public AttributeHandler
    {
    private:
        SvxFrameDirection   m_eParagraphDirection;
        SvxAdjust           m_eDefaultAdjustment;
        SvxAdjust           m_eOppositeDefaultAdjustment;

    public:
        explicit ParagraphDirectionHandler( AttributeId _nAttributeId );

    public:
        virtual     AttributeCheckState implGetCheckState( const SfxPoolItem& _rItem ) const SAL_OVERRIDE;
        virtual     void                executeAttribute( const SfxItemSet& _rCurrentAttribs, SfxItemSet& _rNewAttribs, const SfxPoolItem* _pAdditionalArg, SvtScriptType _nForScriptType ) const SAL_OVERRIDE;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_RTATTRIBUTEHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
