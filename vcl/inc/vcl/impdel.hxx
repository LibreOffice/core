/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impdel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 15:05:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
