/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <editeng/fieldupdater.hxx>
#include <editeng/flditem.hxx>
#include "editobj2.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

using namespace com::sun::star;

namespace editeng {

class FieldUpdaterImpl
{
    EditTextObjectImpl& mrObj;
public:
    explicit FieldUpdaterImpl(EditTextObject const & rObj) : mrObj(*rObj.mpImpl) {}

    void updateTableFields(int nTab)
    {
        SfxItemPool* pPool = mrObj.GetPool();
        EditTextObjectImpl::ContentInfosType& rContents = mrObj.GetContents();
        for (std::unique_ptr<ContentInfo> & i : rContents)
        {
            ContentInfo& rContent = *i;
            for (XEditAttribute & rAttr : rContent.GetCharAttribs())
            {
                const SfxPoolItem* pItem = rAttr.GetItem();
                if (pItem->Which() != EE_FEATURE_FIELD)
                    // This is not a field item.
                    continue;

                const SvxFieldItem* pFI = static_cast<const SvxFieldItem*>(pItem);
                const SvxFieldData* pData = pFI->GetField();
                if (pData->GetClassId() != text::textfield::Type::TABLE)
                    // This is not a table field.
                    continue;

                // Create a new table field with the new ID, and set it to the
                // attribute object.
                SvxFieldItem aNewItem(SvxTableField(nTab), EE_FEATURE_FIELD);
                rAttr.SetItem(pPool->Put(aNewItem));
            }
        }
    }
};

FieldUpdater::FieldUpdater(EditTextObject const & rObj) : mpImpl(new FieldUpdaterImpl(rObj)) {}
FieldUpdater::FieldUpdater(const FieldUpdater& r) : mpImpl(new FieldUpdaterImpl(*r.mpImpl)) {}

FieldUpdater::~FieldUpdater()
{
}

void FieldUpdater::updateTableFields(int nTab)
{
    mpImpl->updateTableFields(nTab);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
