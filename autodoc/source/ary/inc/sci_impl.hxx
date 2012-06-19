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
    typedef typename source::const_iterator source_iterator;

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
class SCI_Map : public StdConstIterator< typename std::map<KEY,VALUE>::value_type >
{
  public:
    typedef std::map<KEY,VALUE>     source;
    typedef typename source::const_iterator source_iterator;

                        SCI_Map(
                            const source &      i_rSource  );
    virtual             ~SCI_Map();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const typename std::map<KEY,VALUE>::value_type *
                        inq_CurElement() const;
    virtual bool        inq_IsSorted() const;

    // DATA
    source_iterator     itRun;
    source_iterator     itEnd;
};


//*************************     SCI_MultiMap      **********************************//

template <class KEY, class VALUE>
class SCI_MultiMap : public StdConstIterator< typename std::multimap<KEY,VALUE>::value_type >
{
  public:
    typedef std::multimap<KEY,VALUE>    source;
    typedef typename source::const_iterator      source_iterator;

                        SCI_MultiMap(
                            const source &      i_rSource  );
                        SCI_MultiMap(
                            source_iterator     i_begin,
                            source_iterator     i_end );
    virtual             ~SCI_MultiMap();

  private:
    // Interface StdConstIterator<>:
    virtual void        do_Advance();
    virtual const typename std::multimap<KEY,VALUE>::value_type *
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
    typedef typename source::const_iterator source_iterator;

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
    typedef typename source::const_iterator  source_iterator;

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
const typename std::map<KEY,VALUE>::value_type *
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
const typename std::multimap<KEY,VALUE>::value_type *
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
