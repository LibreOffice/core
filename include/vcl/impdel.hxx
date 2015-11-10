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

#ifndef INCLUDED_VCL_IMPDEL_HXX
#define INCLUDED_VCL_IMPDEL_HXX

#include <list>

namespace vcl
{

class DeletionListener;

class DeletionNotifier
{
    std::list< DeletionListener* > m_aListeners;
    protected:
    DeletionNotifier() {}

    ~DeletionNotifier()
    { notifyDelete(); }

    inline void notifyDelete();

    public:
    void addDel( DeletionListener* pListener )
    { m_aListeners.push_back( pListener ); }

    void removeDel( DeletionListener* pListener )
    { m_aListeners.remove( pListener ); }
};

class DeletionListener
{
    DeletionNotifier*  m_pNotifier;
    public:
    DeletionListener( DeletionNotifier* pNotifier )
    :  m_pNotifier( pNotifier )
       {
           if( m_pNotifier )
               m_pNotifier->addDel( this );
       }
   ~DeletionListener()
   {
       if( m_pNotifier )
           m_pNotifier->removeDel( this );
   }
   void deleted() { m_pNotifier = nullptr; }
   bool isDeleted() const { return (m_pNotifier == nullptr); }
};

inline void DeletionNotifier::notifyDelete()
{
    for( std::list< DeletionListener* >::const_iterator it =
            m_aListeners.begin(); it != m_aListeners.end(); ++it )
       (*it)->deleted();

    m_aListeners.clear();
}

} // namespace vcl

#endif // INCLUDED_VCL_IMPDEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
