/*************************************************************************
 *
 *  $RCSfile: flddropdown.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:02:31 $
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
#ifndef _FLDDROPDOWN_HXX
#define _FLDDROPDOWN_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <fldbas.hxx>
#include <vector>

using namespace ::com::sun::star::uno;
using namespace rtl;
using namespace std;

/**
    Field type for dropdown boxes.
*/
class SwDropDownFieldType : public SwFieldType
{
public:
    /**
       Constructor
    */
    SwDropDownFieldType();

    /**
       Destructor
    */
    virtual ~SwDropDownFieldType();

    /**
       Create a copy of this field type.

       @return a copy of this type
    */
    virtual SwFieldType * Copy () const;
};

/**
   Dropdown field.

   The dropdown field contains a list of strings. At most one of them
   can be selected.
*/
class SwDropDownField : public SwField
{
    /**
       the possible values (aka items) of the dropdown box
    */
    vector<String> aValues;

    /**
      the selected item
    */
    String aSelectedItem;

    /**
      the name of the field
    */
    String aName;

public:
    /**
       Constructor

       @param pTyp field type for this field
    */
    SwDropDownField(SwFieldType * pTyp);

    /**
       Copy constructor

       @param rSrc dropdown field to copy
    */
    SwDropDownField(const SwDropDownField & rSrc);

    /**
       Destructor
    */
    virtual ~SwDropDownField();

    /**
       Expands the field.

       The expanded value of the field is the value of the selected
       item. If no item is selected, an empty string is returned.

       @return the expanded value of the field
    */
    virtual String Expand() const;

    /**
       Creates a copy of this field.

       @return the copy of this field
    */
    virtual SwField * Copy() const;

    /**
       Returns the selected value.

       @see Expand

       @return the selected value
    */
    virtual const String & GetPar1() const;

    /**
       Returns the name of the field.

       @return the name of the field
    */
    virtual String GetPar2() const;

    /**
       Sets the selected value.

       If rStr is an item of the field that item will be
       selected. Otherwise no item will be selected, i.e. the
       resulting selection will be empty.
    */
    virtual void SetPar1(const String & rStr);

    /**
       Sets the name of the field.

       @param rStr the new name of the field
    */
    virtual void SetPar2(const String & rStr);

    /**
       Sets the items of the dropdown box.

       After setting the items the selection will be empty.

       @param rItems the new items
    */
    void SetItems(const vector<String> & rItems);

    /**
       Sets the items of the dropdown box.

       After setting the items the selection will be empty.

       @param rItems the new items
    */
    void SetItems(const Sequence<OUString> & rItems);

    /**
        Returns the items of the dropdown box.

        @return the items of the dropdown box
    */
    Sequence<OUString> GetItemSequence() const;

    /**
       Returns the selected item.

       @return the selected item
    */
    const String & GetSelectedItem() const;

    /**
       Returns the name of the field.

       @return the name of the field
    */
    const String & GetName() const;

    /**
       Sets the selected item.

       If rItem is found in this dropdown field it is selected. If
       rItem is not found the selection will be empty.

       @param rItem the item to be set

       @retval TRUE the selected item was successfully set
       @retval TRUE failure (empty selection)
    */
    BOOL SetSelectedItem(const String & rItem);

    /**
       Sets the name of the field.

       @param rName the new name of the field
    */
    void SetName(const String & rName);

    /**
       API: Gets a property value from the dropdown field.

       @param rVal return value
       @param nMId
          - FIELD_PROP_PAR1 Get selected item (String)
          - FIELD_PROP_STRINGS Get all items (Sequence)
    */
    virtual BOOL QueryValue(Any &rVal, BYTE nMId) const;

    /**
       API: Sets a property value on the dropdown field.

       @param rVal value to set
       @param nMId
          - FIELD_PROP_PAR1 Set selected item (String)
          - FIELD_PROP_STRINGS Set all items (Sequence)
    */
        virtual BOOL PutValue(const Any &rVal, BYTE nMId);
};

#endif
