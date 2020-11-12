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
#ifndef INCLUDED_SW_INC_FLDDROPDOWN_HXX
#define INCLUDED_SW_INC_FLDDROPDOWN_HXX

#include <com/sun/star/uno/Sequence.h>
#include "swdllapi.h"
#include "fldbas.hxx"

#include <vector>

/**
    Field type for dropdown boxes.
*/
class SAL_DLLPUBLIC_RTTI SwDropDownFieldType final : public SwFieldType
{
public:
    /**
       Constructor
    */
    SwDropDownFieldType();

    /**
       Destructor
    */
    virtual ~SwDropDownFieldType() override;

    /**
       Create a copy of this field type.

       @return a copy of this type
    */
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

/**
   Dropdown field.

   The dropdown field contains a list of strings. At most one of them
   can be selected.
*/
class SW_DLLPUBLIC SwDropDownField final : public SwField
{
    /**
       the possible values (aka items) of the dropdown box
    */
    std::vector<OUString> m_aValues;

    /**
      the selected item
    */
    OUString m_aSelectedItem;

    /**
      the name of the field
    */
    OUString m_aName;

    /**
       help text
     */
    OUString m_aHelp;

    /**
       tool tip string
     */
    OUString m_aToolTip;

    /**
       Expands the field.

       The expanded value of the field is the value of the selected
       item. If no item is selected, an empty string is returned.

       @return the expanded value of the field
    */
    virtual OUString ExpandImpl(SwRootFrame const* pLayout) const override;

    /**
       Creates a copy of this field.

       @return the copy of this field
    */
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    /**
       Constructor

       @param pTyp field type for this field
    */
    SwDropDownField(SwFieldType* pTyp);

    /**
       Copy constructor

       @param rSrc dropdown field to copy
    */
    SwDropDownField(const SwDropDownField& rSrc);

    /**
       Destructor
    */
    virtual ~SwDropDownField() override;

    /**
       Returns the selected value.

       @see Expand

       @return the selected value
    */
    virtual OUString GetPar1() const override;

    /**
       Returns the name of the field.

       @return the name of the field
    */
    virtual OUString GetPar2() const override;

    /**
       Sets the selected value.

       If rStr is an item of the field that item will be
       selected. Otherwise no item will be selected, i.e. the
       resulting selection will be empty.
    */
    virtual void SetPar1(const OUString& rStr) override;

    /**
       Sets the name of the field.

       @param rStr the new name of the field
    */
    virtual void SetPar2(const OUString& rStr) override;

    /**
       Sets the items of the dropdown box.

       After setting the items the selection will be empty.

       @param rItems the new items
    */
    void SetItems(const std::vector<OUString>& rItems);

    /**
       Sets the items of the dropdown box.

       After setting the items the selection will be empty.

       @param rItems the new items
    */
    void SetItems(const css::uno::Sequence<OUString>& rItems);

    /**
        Returns the items of the dropdown box.

        @return the items of the dropdown box
    */
    css::uno::Sequence<OUString> GetItemSequence() const;

    /**
       Returns the selected item.

       @return the selected item
    */
    const OUString& GetSelectedItem() const { return m_aSelectedItem; }

    /**
       Returns the name of the field.

       @return the name of the field
    */
    const OUString& GetName() const { return m_aName; }

    /**
       Returns the help text of the field.

       @return the help text of the field
    */
    const OUString& GetHelp() const { return m_aHelp; }

    /**
       Returns the tool tip of the field.

       @return the tool tip of the field
     */
    const OUString& GetToolTip() const { return m_aToolTip; }

    /**
       Sets the selected item.

       If rItem is found in this dropdown field it is selected. If
       rItem is not found the selection will be empty.

       @param rItem the item to be set
    */
    void SetSelectedItem(const OUString& rItem);

    /**
       Sets the name of the field.

       @param rName the new name of the field
    */
    void SetName(const OUString& rName);

    /**
       Sets the help text of the field.

       @param rHelp    the help text
    */
    void SetHelp(const OUString& rHelp);

    /**
       Sets the tool tip of the field.

       @param rToolTip  the tool tip
    */
    void SetToolTip(const OUString& rToolTip);

    /**
       API: Gets a property value from the dropdown field.

       @param rVal return value
       @param nMId
          - FIELD_PROP_PAR1 Get selected item (String)
          - FIELD_PROP_STRINGS Get all items (Sequence)
          - FIELD_PROP_PAR3 Get the help text of the field.
          - FIELD_PROP_PAR4 Get the tool tip of the field.
    */
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt16 nWhichId) const override;

    /**
       API: Sets a property value on the dropdown field.

       @param rVal value to set
       @param nMId
          - FIELD_PROP_PAR1 Set selected item (String)
          - FIELD_PROP_STRINGS Set all items (Sequence)
          - FIELD_PROP_PAR3  Set the help text of the field.
          - FIELD_PROP_PAR4  Set the tool tip of the field.
    */
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt16 nWhichId) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
