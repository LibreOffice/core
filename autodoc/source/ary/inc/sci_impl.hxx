/*************************************************************************
 *
 *  $RCSfile: sci_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 14:11:34 $
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

#ifndef ARY_SCI_IMPL_HXX
#define ARY_SCI_IMPL_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/stdconstiter.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{


//*************************     SCI_Vector      **********************************//

template <class ELEM>
class SCI_Vector : public StdConstIterator<ELEM>
{
  public:
    typedef std::vector<ELEM>       source;
    typedef source::const_iterator  source_iterator;

                        SCI_Vector(
                            const source &      i_rSource  );
    virtual             ~SCI_Vector();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const ELEM *
                        inq_CurElement() const;
    virtual bool        inq_IsSorted() const;

    // DATA
    source_iterator     itRun;
    source_iterator     itEnd;
};



//*************************     SCI_Map      **********************************//

template <class KEY, class VALUE>
class SCI_Map : public StdConstIterator< std::map<KEY,VALUE>::value_type >
{
  public:
    typedef std::map<KEY,VALUE>     source;
    typedef source::const_iterator  source_iterator;

                        SCI_Map(
                            const source &      i_rSource  );
    virtual             ~SCI_Map();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const std::map<KEY,VALUE>::value_type *
                        inq_CurElement() const;
    virtual bool        inq_IsSorted() const;

    // DATA
    source_iterator     itRun;
    source_iterator     itEnd;
};


//*************************     SCI_MultiMap      **********************************//

template <class KEY, class VALUE>
class SCI_MultiMap : public StdConstIterator< std::multimap<KEY,VALUE>::value_type >
{
  public:
    typedef std::multimap<KEY,VALUE>    source;
    typedef source::const_iterator      source_iterator;

                        SCI_MultiMap(
                            const source &      i_rSource  );
                        SCI_MultiMap(
                            source_iterator     i_begin,
                            source_iterator     i_end );
    virtual             ~SCI_MultiMap();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const std::multimap<KEY,VALUE>::value_type *
                        inq_CurElement() const;
    virtual bool        inq_IsSorted() const;

    // DATA
    source_iterator     itRun;
    source_iterator     itEnd;
};



//*************************     SCI_Set     **********************************//


template <class TYPES>
class SCI_Set : public StdConstIterator<typename TYPES::element_type>
{
  public:
    typedef typename TYPES::element_type    element;
    typedef typename TYPES::sort_type       sorter;
    typedef std::set<element, sorter>       source;
    typedef source::const_iterator          source_iterator;

                        SCI_Set(
                            const source &      i_rSource  );
    virtual             ~SCI_Set();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const element *
                        inq_CurElement() const;
    virtual bool        inq_IsSorted() const;

    // DATA
    source_iterator     itRun;
    source_iterator     itEnd;
};

//*************************     SCI_DataInMap    **********************************//

template <class KEY, class VALUE>
class SCI_DataInMap : public StdConstIterator<VALUE>
{
  public:
    typedef std::map<KEY,VALUE>     source;
    typedef source::const_iterator  source_iterator;

                        SCI_DataInMap(
                            const source &      i_rSource  );
    virtual             ~SCI_DataInMap();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const VALUE *
                        inq_CurElement() const;
    virtual bool        inq_IsSorted() const;

    // DATA
    source_iterator     itRun;
    source_iterator     itEnd;
};





//********************************************************************//


// IMPLEMENTATION

template <class ELEM>
SCI_Vector<ELEM>::SCI_Vector( const source & i_rSource  )
    :   itRun(i_rSource.begin()),
        itEnd(i_rSource.end())
{
}

template <class ELEM>
SCI_Vector<ELEM>::~SCI_Vector()
{
}


template <class ELEM>
void
SCI_Vector<ELEM>::do_Advance()
{
    if (itRun != itEnd)
        ++itRun;
}

template <class ELEM>
const ELEM *
SCI_Vector<ELEM>::inq_CurElement() const
{
    if (itRun != itEnd)
        return &(*itRun);
    return 0;
}

template <class ELEM>
bool
SCI_Vector<ELEM>::inq_IsSorted() const
{
    return false;
}




template <class KEY, class VALUE>
SCI_Map<KEY,VALUE>::SCI_Map( const source & i_rSource  )
    :   itRun(i_rSource.begin()),
        itEnd(i_rSource.end())
{
}

template <class KEY, class VALUE>
SCI_Map<KEY,VALUE>::~SCI_Map()
{
}

template <class KEY, class VALUE>
void
SCI_Map<KEY,VALUE>::do_Advance()
{
    if (itRun != itEnd)
        ++itRun;
}

template <class KEY, class VALUE>
const std::map<KEY,VALUE>::value_type *
SCI_Map<KEY,VALUE>::inq_CurElement() const
{
    if (itRun != itEnd)
        return &(*itRun);
    return 0;
}


template <class KEY, class VALUE>
bool
SCI_Map<KEY,VALUE>::inq_IsSorted() const
{
    return true;
}







template <class KEY, class VALUE>
SCI_MultiMap<KEY,VALUE>::SCI_MultiMap( const source & i_rSource  )
    :   itRun(i_rSource.begin()),
        itEnd(i_rSource.end())
{
}

template <class KEY, class VALUE>
SCI_MultiMap<KEY,VALUE>::SCI_MultiMap( source_iterator i_begin,
                                       source_iterator i_end )
    :   itRun(i_begin),
        itEnd(i_end)
{
}

template <class KEY, class VALUE>
SCI_MultiMap<KEY,VALUE>::~SCI_MultiMap()
{
}

template <class KEY, class VALUE>
void
SCI_MultiMap<KEY,VALUE>::do_Advance()
{
    if (itRun != itEnd)
        ++itRun;
}

template <class KEY, class VALUE>
const std::multimap<KEY,VALUE>::value_type *
SCI_MultiMap<KEY,VALUE>::inq_CurElement() const
{
    if (itRun != itEnd)
        return &(*itRun);
    return 0;
}


template <class KEY, class VALUE>
bool
SCI_MultiMap<KEY,VALUE>::inq_IsSorted() const
{
    return true;
}








template <class ELEM>
SCI_Set<ELEM>::SCI_Set( const source & i_rSource  )
    :   itRun(i_rSource.begin()),
        itEnd(i_rSource.end())
{
}

template <class ELEM>
SCI_Set<ELEM>::~SCI_Set()
{
}


template <class ELEM>
void
SCI_Set<ELEM>::do_Advance()
{
    if (itRun != itEnd)
        ++itRun;
}

template <class ELEM>
const typename SCI_Set<ELEM>::element *
SCI_Set<ELEM>::inq_CurElement() const
{
    if (itRun != itEnd)
        return &(*itRun);
    return 0;
}

template <class ELEM>
bool
SCI_Set<ELEM>::inq_IsSorted() const
{
    return true;
}







template <class KEY, class VALUE>
SCI_DataInMap<KEY,VALUE>::SCI_DataInMap( const source & i_rSource  )
    :   itRun(i_rSource.begin()),
        itEnd(i_rSource.end())
{
}

template <class KEY, class VALUE>
SCI_DataInMap<KEY,VALUE>::~SCI_DataInMap()
{
}

template <class KEY, class VALUE>
void
SCI_DataInMap<KEY,VALUE>::do_Advance()
{
    if (itRun != itEnd)
        ++itRun;
}

template <class KEY, class VALUE>
const VALUE *
SCI_DataInMap<KEY,VALUE>::inq_CurElement() const
{
    if (itRun != itEnd)
        return &(*itRun).second;
    return 0;
}


template <class KEY, class VALUE>
bool
SCI_DataInMap<KEY,VALUE>::inq_IsSorted() const
{
    return true;
}







}   // namespace ary


#endif
