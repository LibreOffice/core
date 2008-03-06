/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicmanagerrepository.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:50:57 $
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

#ifndef BASICMANAGERREPOSITORY_HXX
#define BASICMANAGERREPOSITORY_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
/** === end UNO includes === **/

class BasicManager;

//........................................................................
namespace basic
{
//........................................................................

    //====================================================================
    //= BasicManagerRepository
    //====================================================================
    /** specifies a callback for instances which are interested in BasicManagers
        created by the BasicManagerRepository.
    */
    class SAL_NO_VTABLE BasicManagerCreationListener
    {
    public:
        /** is called when a BasicManager has been created

            @param  _rxForDocument
                denotes the document for which the BasicManager has been created. If this is <NULL/>,
                then the BasicManager is the application-wide BasicManager.

            @param  _pBasicManager
                denotes the BasicManager which has been created. The listener might for instance
                decide to add global variables to it, or otherwise initialize it.
        */
        virtual void onBasicManagerCreated(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxForDocument,
            BasicManager& _rBasicManager
        ) = 0;
    };

    //====================================================================
    //= BasicManagerRepository
    //====================================================================
    class BasicManagerRepository
    {
    public:
        /** returns the BasicManager belonging to the given document

            If the BasicManager does not yet exist, it is created. In this case, if the application's
            BasicManager does not yet exist, it is also created. This is necessary since
            the application's BasicManager acts as parent for all document's BasicManagers.

            If you're interested in this case - the implicit creation of the application's BasicManager -,
            then you need to register as BasicManagerCreationListener.

            @param _rxDocumentModel
                denotes the document model whose BasicManager is to be retrieved. Must not be <NULL/>.
                The document should support the XDocumentInfoSupplier interface, for retrieving
                its title, which is needed in some error conditions.
                Also it <em>must</em> support the XStorageBasedDocument interface, since we
                must be able to retrieve the document's storage. If this interface is <em>not</em>
                supported, creating a new BasicManager will certainly fail.

            @return
                the BasicManager for this model.

            @attention
                The returned BasicManager instances is owned by the repository. In particular,
                you are not allowed to delete it. Instead, the given model is observed: As soon
                as it's closed, the associated BasicManager is deleted.
        */
        static BasicManager* getDocumentBasicManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocumentModel
        );

        /** returns the application-wide BasicManager

        @param _bCreate
            determines whether the BasicManager should be created (<TRUE/>) if it
            does not yet exist.

        @attention
            If the BasicManager is newly created, then it is still owned by the repository.
            In particular, you are not allowed to delete it. Instead, call resetApplicationBasicManager
            to release the BasicManager.
        */
        static BasicManager* getApplicationBasicManager( bool _bCreate );

        /** resets the application-wide BasicManager to <NULL/>
        */
        static void resetApplicationBasicManager();

        /** registers a BasicManagerCreationListener instance which is notified whenever
            the repository creates a BasicManager instance.

            Note that this listener is <em>not</em> called when somebody else
            creates BasicManager instances.

            If the same listener is registered multiple times, it is also notified
            multiple times, and needs to be revoked once for each registration.
        */
        static  void    registerCreationListener(
                BasicManagerCreationListener& _rListener
            );

        /** reveokes a BasicManagerCreationListener instance which has previously
            been registered to be notified about created BasicManager instances.
        */
        static  void    revokeCreationListener(
                BasicManagerCreationListener& _rListener
            );
    };

//........................................................................
} // namespace basic
//........................................................................

#endif // BASICMANAGERREPOSITORY_HXX

