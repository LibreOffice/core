/*************************************************************************
 *
 *  $RCSfile: TSortIndex.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-29 12:18:15 $
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

#include "TKeyValue.hxx"

namespace connectivity
{
    typedef enum
    {
        SQL_ORDERBYKEY_NONE,        // Nicht sortieren
        SQL_ORDERBYKEY_DOUBLE,      // Numerischer Key
        SQL_ORDERBYKEY_STRING       // String Key
    } OKeyType;

    typedef enum
    {
        SQL_ASC     = 1,
        SQL_DESC    = -1
    } TAscendingOrder;

    class OKeySet;
    class OKeyValue;

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


        sal_Bool AddKeyValue(OKeyValue * pKeyValue);
                                                            // TRUE, wenn erfolgreich hinzugefuegt, FALSE bei Ueberschreitung
                                                            // der Index-Kapazitaet.
                                                            // pKeyValue wird beim Zerstoeren des Index automatisch freigegeben.

        void Freeze();                                      // "Einfrieren" des Index:
                                                            // Vor "Freeze" duerfen Count() und Get() nicht gerufen werden,
                                                            // nach "Freeze" darf dafuer Add() nicht mehr gerufen werden.

        ::vos::ORef<OKeySet> CreateKeySet();



        sal_Bool IsFrozen() const { return m_bFrozen; }         // TRUE nach Aufruf von Freeze()

        sal_Int32 Count()   const { return m_aKeyValues.size(); }       // Anzahl Key/Value-Paare im Index
        sal_Int32 GetValue(sal_Int32 nPos) const;           // Value an Position nPos (1..n) [sortierter Zugriff].

        inline const ::std::vector<OKeyType>& getKeyType() const { return m_aKeyType; }
        inline sal_Int16 getAscending(::std::vector<sal_Int16>::size_type _nPos) const { return m_aAscending[_nPos]; }

    };

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
