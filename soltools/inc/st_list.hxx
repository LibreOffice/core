/*************************************************************************
 *
 *  $RCSfile: st_list.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:08:11 $
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

#ifndef SOLTOOLS_ST_LIST_HXX
#define SOLTOOLS_ST_LIST_HXX

#include <string.h>
#include <iostream>
#include <stdlib.h>

template <class XX>
class ST_List            /// Soltools-List.
{
  public :
    typedef XX *        iterator;
    typedef const XX *  const_iterator;

    // LIFECYCLE
                        ST_List();
                        ST_List(
                            const ST_List<XX> & i_rList );
    virtual             ~ST_List() { }

    // OPERATORS
    ST_List<XX> &       operator=(
                            const ST_List<XX> & i_rList );

    const XX &          operator[](
                            unsigned            n) const
                                                { return elem(n); }
    XX &                operator[](
                            unsigned            n)
                                                { return elem(n); }
    // OPERATIONS
    void                reserve(
                            unsigned            i_nSize )
                                                { alloc(i_nSize,true); }
    void                insert(
                            iterator            i_aPos,
                            const XX &          elem )
                                                { Insert(i_aPos-begin(), elem); }
    virtual void        Insert(
                            unsigned            pos,
                            const XX &          elem );
    void                push_back(
                            const XX &          elem)
                                                { Insert(size(),elem); }
    void                remove(
                            iterator            i_aPos )
                                                { Remove(i_aPos-begin()); }
    virtual void        Remove(
                            unsigned            pos );
    void                pop_back()              { Remove(size()-1); }
    void                erase_all()             { while (size()) Remove(size()-1); }

    // INQUIRY
    const_iterator      begin() const           { return &inhalt[0]; }
    const_iterator      end() const             { return &inhalt[len]; }

    const XX &          front() const           { return elem(0); }
    const XX &          back() const            { return elem(len-1); }

    unsigned            size() const            { return len; }
    unsigned            space() const           { return allocated; }
    bool                is_valid_index(
                            unsigned            n) const
                                                { return n < len; }
    // ACCESS
    iterator            begin()                 { return &inhalt[0]; }
    iterator            end()                   { return &inhalt[len]; }

    XX &                front()                 { return elem(0); }
    XX &                back()                  { return elem(len-1); }

  protected:
    void                checkSize(
                            unsigned            newLength);
    void                alloc(
                            unsigned            newSpace,
                            bool                re = false );

    const XX &          elem(
                            unsigned            n ) const
                                                { return inhalt[n]; }
    XX &                elem(
                            unsigned            n )
                                                { return inhalt[n]; }
  // DATA
    XX *                inhalt;
    unsigned            len;
    unsigned            allocated;
};



template <class XY>
class DynamicList : public ST_List< XY* >
{
  public:
                        DynamicList();
                        DynamicList(
                            const DynamicList<XY> &
                                                i_rList );
    virtual             ~DynamicList();         /// Deletes all member pointers

    DynamicList<XY> &   operator=(
                            const DynamicList<XY> &
                                                i_rList );

    virtual void        Insert(
                            unsigned            pos,
                            XY * const &        elem );
    virtual void        Remove(
                            unsigned            pos );
};



template <class XX>
ST_List<XX>::ST_List()
    :   inhalt(0),
        len(0),
        allocated(0)
{
    alloc(1);
}

template <class XX>
ST_List<XX>::ST_List( const ST_List<XX> & i_rList )
    :   inhalt(0),
        len(0),
        allocated(0)
{
    alloc(i_rList.size());

    for ( const_iterator it = i_rList.begin();
          it != i_rList.end();
          ++it )
    {
         push_back(*it);
    }
}

template <class XX>
ST_List<XX> &
ST_List<XX>::operator=( const ST_List<XX> & i_rList )
{
    for ( const_iterator it = i_rList.begin();
          it != i_rList.end();
          ++it )
    {
         push_back(*it);
    }
    return *this;
}

template <class XX>
void
ST_List<XX>::Insert(unsigned pos, const XX & elem)
{
    if ( pos > len )
      return;

    checkSize(len+2);
    for ( unsigned p = len; p > pos; --p)
    {
        inhalt[p] = inhalt[p-1];
    }
    inhalt[pos] = elem;
    len++;
}


template <class XX>
void
ST_List<XX>::Remove(unsigned pos)
{
    if ( pos >= len )
      return;
    len--;
    for ( unsigned p = pos; p < len; ++p)
    {
        inhalt[p] = inhalt[p+1];
    }
}


// Protected:
template <class XX>
void
ST_List<XX>::checkSize(unsigned newLength)
{
   // neuen Platzbedarf pruefen:
   unsigned newSpace = space();
   if (newLength >= newSpace)
   {
      if (!newSpace)
         newSpace = 1;
      const unsigned nBorder = 2000000000;
      while(newLength >= newSpace)
      {
        if (newSpace < nBorder)
            newSpace <<= 1;
        else
        {
            std::cerr << "List becomes too big" << std::endl;
            exit(1);
        }
      }
   }

   // Veraenderung ?:
   if (newSpace != space())
      alloc(newSpace,true);
}

template <class XX>
void
ST_List<XX>::alloc( unsigned            newSpace,
                    bool               re )
{
    XX * pNew = new XX[newSpace];

    if (inhalt != 0)
    {
        if (re)
        {
            for (unsigned i = 0; i < len; ++i)
            {
                pNew[i] = inhalt[i];
            }  // end for
        }
        delete [] inhalt;
    }

    inhalt = pNew;
    allocated = newSpace;
}


template <class XY>
DynamicList<XY>::DynamicList()
{
}

template <class XY>
DynamicList<XY>::DynamicList( const DynamicList<XY> & i_rList )
    :   ST_List< XY* >(i_rList)
{
    for ( typename DynamicList<XY>::iterator it = this->begin();
          it != DynamicList<XY>::end();
          ++it )
    {
        // Copying the contents the pointers point at:
         (*it) = new XY( *(*it) );
    }
}

template <class XY>
DynamicList<XY>::~DynamicList()
{
    this->erase_all();
}

template <class XY>
DynamicList<XY> &
DynamicList<XY>::operator=( const DynamicList<XY> & i_rList )
{
    for ( typename DynamicList<XY>::const_iterator it = i_rList.begin();
          it != i_rList.end();
          ++it )
    {
         push_back( new XY(*(*it)) );
    }
    return *this;
}


template <class XY>
void
DynamicList<XY>::Insert(unsigned pos, XY * const & elem)
{
    if ( pos > this->len )
      return;

    checkSize(DynamicList<XY>::len+2);
    memmove( DynamicList<XY>::inhalt+pos+1, DynamicList<XY>::inhalt+pos, (DynamicList<XY>::len-pos) * sizeof(XY*) );
    this->inhalt[pos] = elem;
    this->len++;
}

template <class XY>
void
DynamicList<XY>::Remove( unsigned pos )
{
    if (!this->is_valid_index(pos) )
        return;
    this->len--;
    delete DynamicList<XY>::inhalt[pos];
    memmove(DynamicList<XY>::inhalt+pos, DynamicList<XY>::inhalt+pos+1, (DynamicList<XY>::len-pos) * sizeof(XY*) );
}



#endif

