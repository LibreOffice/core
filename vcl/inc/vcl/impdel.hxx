/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impdel.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

 #ifndef _VCL_IMPDEL_HXX
 #define _VCL_IMPDEL_HXX

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
    void deleted() { m_pNotifier = NULL; }
    bool isDeleted() const { return (m_pNotifier == NULL); }
 };

 inline void DeletionNotifier::notifyDelete()
 {
     for( std::list< DeletionListener* >::const_iterator it =
             m_aListeners.begin(); it != m_aListeners.end(); ++it )
        (*it)->deleted();

     m_aListeners.clear();
 }

 } // namespace vcl

 #endif // _VCL_IMPDEL_HXX
