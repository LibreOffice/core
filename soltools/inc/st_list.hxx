/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                            const XX &          elem_ )
                                                { Insert(i_aPos-begin(), elem_); }
    virtual void        Insert(
                            unsigned            pos,
                            const XX &          elem );
    void                push_back(
                            const XX &          elem_)
                                                { Insert(size(),elem_); }
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
ST_List<XX>::Insert(unsigned pos, const XX & elem_)
{
    if ( pos > len )
      return;

    checkSize(len+2);
    for ( unsigned p = len; p > pos; --p)
    {
        inhalt[p] = inhalt[p-1];
    }
    inhalt[pos] = elem_;
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
DynamicList<XY>::Insert(unsigned pos, XY * const & elem_)
{
    if ( pos > this->len )
      return;

    checkSize(DynamicList<XY>::len+2);
    memmove( DynamicList<XY>::inhalt+pos+1, DynamicList<XY>::inhalt+pos, (DynamicList<XY>::len-pos) * sizeof(XY*) );
    this->inhalt[pos] = elem_;
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

