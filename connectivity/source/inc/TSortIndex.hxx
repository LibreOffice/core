/*************************************************************************
 *
 *  $RCSfile: TSortIndex.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-05 07:03:02 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef CONNECTIVITY_TSORTINDEX_HXX
#define CONNECTIVITY_TSORTINDEX_HXX

#ifndef CONNECTIVITY_TKEYVALUE_HXX
#include "TKeyValue.hxx"
#endif

namespace connectivity
{
    typedef enum
    {
        SQL_ORDERBYKEY_NONE,        // do not sort
        SQL_ORDERBYKEY_DOUBLE,      // numeric key
        SQL_ORDERBYKEY_STRING       // String Key
    } OKeyType;

    typedef enum
    {
        SQL_ASC     = 1,            // ascending
        SQL_DESC    = -1            // otherwise
    } TAscendingOrder;

    class OKeySet;
    class OKeyValue;                // simple class which holds a sal_Int32 and a ::std::vector<ORowSetValueDecoratorRef>

    /**
        The class OSortIndex can be used to implement a sorted index.
        This can depend on the fields which should be sorted.
    */
    class OSortIndex
    {
    public:
        typedef ::std::vector< ::std::pair<sal_Int32,OKeyValue*> >  TIntValuePairVector;
        typedef ::std::vector<OKeyType>                             TKeyTypeVector;

    private:
        TIntValuePairVector         m_aKeyValues;
        TKeyTypeVector              m_aKeyType;
        ::std::vector<sal_Int16>    m_aAscending;
        sal_Bool                    m_bFrozen;

    public:

        OSortIndex( const ::std::vector<OKeyType>& _aKeyType,
                    const ::std::vector<sal_Int16>& _aAscending);

        ~OSortIndex();

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t nSize,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void * pMem,void* _pHint ) SAL_THROW( () )
            {  }


        /**
            AddKeyValue appends a new value.
            @param
                pKeyValue   the keyvalue to be appended
            ATTENTION: when the sortindex is already frozen the parameter will be deleted
        */
        void AddKeyValue(OKeyValue * pKeyValue);

        /**
            Freeze freezes the sortindex so that new values could only be appended by their value
        */
        void Freeze();

        /**
            CreateKeySet creates the keyset which vaalues could be used to travel in your table/result
            The returned keyset is frozen.
        */
        ::vos::ORef<OKeySet> CreateKeySet();



        // look at the name
        sal_Bool IsFrozen() const { return m_bFrozen; }
        // returns the current size of the keyvalues
        sal_Int32 Count()   const { return m_aKeyValues.size(); }
        /** GetValue returns the value at position nPos (1..n) [sorted access].
            It only allowed to call this method after the sortindex has been frozen.
        */

        sal_Int32 GetValue(sal_Int32 nPos) const;

        inline const ::std::vector<OKeyType>& getKeyType() const { return m_aKeyType; }
        inline sal_Int16 getAscending(::std::vector<sal_Int16>::size_type _nPos) const { return m_aAscending[_nPos]; }

    };

    /**
        The class OKeySet is a refcountable vector which also has a state.
        This state gives information about if the keyset is fixed.
    */
    class OKeySet : public ORefVector<sal_Int32>
    {
        sal_Bool m_bFrozen;
    public:
        OKeySet()
            : ORefVector<sal_Int32>()
            , m_bFrozen(sal_False){}
        OKeySet(size_type _nSize)
            : ORefVector<sal_Int32>(_nSize)
            , m_bFrozen(sal_False){}

        sal_Bool    isFrozen() const                        { return m_bFrozen; }
        void        setFrozen(sal_Bool _bFrozen=sal_True)   { m_bFrozen = _bFrozen; }
    };
}
#endif // CONNECTIVITY_TSORTINDEX_HXX
