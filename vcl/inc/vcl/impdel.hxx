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
