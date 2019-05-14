/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX
#define INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX

#include <item/base/ItemBuffered.hxx>
#include <rtl/ustring.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for CntUnencodedStringItem
    // this is a helper base class, so it has *no* method
    //     static ItemControlBlock& GetStaticItemControlBlock();
    // and also no public constructor (!), but implements all the
    // tooling methods for Items using a sal_Int16 internally.
    // I checked if rtl::OUString nowadays already does some
    // office-wide runtime matching and buffering to hold the
    // string data only once, but this is not the case. Construction
    // is optimized, but no shared data usage. Thus, use ItemBuffered
    // as base class and implement shared ItemData for rtl::OUString
    // here now to always only have one instance for CntOUString-based
    // derivations. This will use ItemAdministrator_set, see *.cxx
    // for details
    //
    //class CntUnencodedStringItem: public SfxPoolItem
    //    class SfxStringItem: public CntUnencodedStringItem
    //        class SvxPageModelItem : public SfxStringItem
    //        class SfxDocumentInfoItem : public SfxStringItem
    //        class SvxPostItAuthorItem: public SfxStringItem
    //        class SvxPostItDateItem: public SfxStringItem
    //        class SvxPostItTextItem: public SfxStringItem
    //        class SvxPostItIdItem: public SfxStringItem
    //        class SdrMeasureFormatStringItem: public SfxStringItem
    //        class NameOrIndex : public SfxStringItem
    //            class XFillBitmapItem : public NameOrIndex
    //            class XColorItem : public NameOrIndex
    //                class XFillColorItem : public XColorItem
    //                class XFormTextShadowColorItem : public XColorItem
    //                class XLineColorItem : public XColorItem
    //                class XSecondaryFillColorItem : public XColorItem
    //            class XFillGradientItem : public NameOrIndex
    //                class XFillFloatTransparenceItem : public XFillGradientItem
    //            class XFillHatchItem : public NameOrIndex
    //            class XLineDashItem : public NameOrIndex
    //            class XLineEndItem : public NameOrIndex
    //            class XLineStartItem : public NameOrIndex
    //        class SfxScriptOrganizerItem : public SfxStringItem
    //        class SdrLayerNameItem: public SfxStringItem
    //        class SwNumRuleItem : public SfxStringItem
    //
    class ITEM_DLLPUBLIC CntOUString : public ItemBuffered
    {
    protected:
        // ItemData class for ref-counted rtl::OUString instances
        class CntOUString_Data : public ItemData
        {
        private:
            rtl::OUString m_aValue;

        protected:
            virtual ItemAdministrator& getItemAdministrator() const override;

        public:
            CntOUString_Data(const rtl::OUString& rValue = rtl::OUString());
            virtual bool operator==(const ItemData& rRef) const override;
            const rtl::OUString& getValue() const { return m_aValue; }
            void setValue(const rtl::OUString& rValue) { m_aValue = rValue; }
        };

    protected:
        // constructor for derived classes that *have* to hand
        // in the to-be-used ItemControlBlock
        CntOUString(ItemControlBlock& rItemControlBlock, const rtl::OUString& rValue = rtl::OUString());

    public:
        CntOUString() = delete;
        virtual bool operator==(const ItemBase&) const override;

        const rtl::OUString& getValue() const;
        void setValue(const rtl::OUString& rValue);

        virtual bool getPresentation(
            SfxItemPresentation,
            MapUnit,
            MapUnit,
            rtl::OUString&,
            const IntlWrapper&) const override;
        virtual bool queryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
        virtual bool putAnyValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

        // from SfxStringItem, was not implemented in CntUnencodedStringItem
        void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTOUSTRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
