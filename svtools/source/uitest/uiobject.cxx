/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uitest/uiobject.hxx>

#include <svtools/valueset.hxx>

ValueSetUIObject::ValueSetUIObject(const VclPtr<ValueSet>& xValueSet)
    : WindowUIObject(xValueSet)
    , mxValueSet(xValueSet)
{
}

ValueSetUIObject::~ValueSetUIObject() {}

void ValueSetUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (!mxValueSet->IsEnabled() || !mxValueSet->IsReallyVisible())
        return;

    if (rAction == "SELECT")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto aPos = rParameters.find("POS");
            OUString aVal = aPos->second;
            sal_Int32 nPos = aVal.toInt32();
            mxValueSet->SelectItem(nPos);
            mxValueSet->Select();
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap ValueSetUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["EntryCount"] = OUString::number(mxValueSet->GetItemCount());

    return aMap;
}

OUString ValueSetUIObject::get_name() const { return "ValueSetUIObject"; }

OUString ValueSetUIObject::get_action(VclEventId /*nEvent*/) const
{
    // No action for this control that trigger item selection after mouse tracking end
    return OUString();
}

std::unique_ptr<UIObject> ValueSetUIObject::create(vcl::Window* pWindow)
{
    ValueSet* pValueSet = dynamic_cast<ValueSet*>(pWindow);
    assert(pValueSet);
    return std::unique_ptr<UIObject>(new ValueSetUIObject(pValueSet));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
