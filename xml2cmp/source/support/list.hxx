/*****************************************************************
*   Project:
*
*   File:                LIST.HXX
*   Description:         Template for Lists
*
*   Created:             21. 8. 1994
 *   Author:             Nikolai Pretzell
******************************************************************/

#ifndef __LISTEN_123456__
#define __LISTEN_123456__

#include <string.h>
#include <ostream.h>
#include <stdlib.h>

template <class XX>
class List
{
  public :
    typedef XX *        iterator;
    typedef const XX *  const_iterator;

    // LIFECYCLE
                        List();
    virtual             ~List() { }

    // OPERATORS
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
    virtual void        insert(
                            unsigned            pos,
                            const XX &          elem );
    void                push_back(
                            const XX &          elem)
                                                { insert(size(),elem); }

    virtual void        remove(
                            unsigned            pos );
    void                pop_back()              { remove(size()-1); }
    void                erase_all()             { while (size()) remove(size()-1); }

    // INQUIRY
    const XX &          front() const           { return elem(0); }
    const XX &          back() const            { return elem(len-1); }

    unsigned            size() const            { return len; }
    unsigned            space() const           { return allocated; }
    bool                is_valid_index(
                            unsigned            n) const
                                                { return n < len; }
    // ACCESS
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

  private:
    // forbidden functions
                        List(const List<XX> & L);
    List<XX> &          operator=(
                            const List<XX> &    L);

};

template <class XY>
class DynamicList : public List<XY*>
{
  public:
    virtual             ~DynamicList();

    virtual void        insert(
                            unsigned            pos,
                            XY * const &        elem );
    virtual void        remove(
                            unsigned            pos );
};



template <class XX>
List<XX>::List()
    :   len(0),
        inhalt(0)
{
    alloc(1);
}


template <class XX>
void
List<XX>::insert(unsigned pos, const XX & elem)
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
List<XX>::remove(unsigned pos)
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
List<XX>::checkSize(unsigned newLength)
{
   // neuen Platzbedarf pruefen:
   unsigned newSpace = space();
   if (newLength > newSpace)
   {
      if (!newSpace)
         newSpace = 1;
      const unsigned nBorder = 65536 / 2;
      while(newLength > newSpace)
      {
        if (newSpace < nBorder)
            newSpace <<= 1;
        else
        {
            cerr << "List becomes too big" << endl;
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
List<XX>::alloc( unsigned           newSpace,
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
DynamicList<XY>::~DynamicList()
{
    erase_all();
}

template <class XY>
void
DynamicList<XY>::insert(unsigned pos, XY * const & elem)
{
    if ( pos > len )
      return;

    checkSize(len+2);
    memmove(inhalt[pos+1], inhalt[pos], (len-pos) * sizeof(XY*) );
    inhalt[pos] = elem;
    len++;
}

template <class XY>
void
DynamicList<XY>::remove( unsigned           pos )
{
    if (!is_valid_index(pos) )
        return;
    len--;
    delete inhalt[pos];
    memmove(inhalt[pos], inhalt[pos+1], (len-pos) * sizeof(XY*) );
}



#endif

