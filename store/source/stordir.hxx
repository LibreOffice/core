/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stordir.hxx,v $
 *
 *  $Revision: 1.1.2.2 $
 *
 *  last change: $Author: mhu $ $Date: 2008/10/17 16:30:17 $
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

#ifndef _STORE_STORDIR_HXX_
#define _STORE_STORDIR_HXX_ "$Revision: 1.1.2.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_TEXTCVT_H_
#include <rtl/textcvt.h>
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _STORE_OBJECT_HXX_
#include "object.hxx"
#endif
#ifndef _STORE_STORBASE_HXX_
#include "storbase.hxx"
#endif
#ifndef _STORE_STORPAGE_HXX_
#include "storpage.hxx"
#endif

namespace store
{

struct OStoreDirectoryPageData;

/*========================================================================
 *
 * OStoreDirectory_Impl interface.
 *
 *======================================================================*/
class OStoreDirectory_Impl : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStoreDirectory_Impl (void);

    /** create (two-phase construction).
     *  @param  pManager [in]
     *  @param  pPath [in]
     *  @param  pName [in]
     *  @param  eAccessMode [in]
     *  @return store_E_None upon success.
     */
    storeError create (
        OStorePageManager *pManager,
        rtl_String        *pPath,
        rtl_String        *pName,
        storeAccessMode    eAccessMode);

    /** iterate.
     *  @param  rFindData [out]
     *  @return store_E_None        upon success,
     *          store_E_NoMoreFiles upon end of iteration.
     */
    storeError iterate (
        storeFindData &rFindData);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    /** Destruction.
     */
    virtual ~OStoreDirectory_Impl (void);

private:
    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template function specialization.
     */
    friend OStoreDirectory_Impl*
    SAL_CALL query<> (IStoreHandle *pHandle, OStoreDirectory_Impl*);

    /** Representation.
     */
    typedef OStoreDirectoryPageData   inode;
    typedef PageHolderObject< inode > inode_holder_type;

    rtl::Reference<OStorePageManager> m_xManager;

    OStorePageDescriptor       m_aDescr;
    sal_uInt32                 m_nPath;
    rtl_TextToUnicodeConverter m_hTextCvt;

    /** Not implemented.
     */
    OStoreDirectory_Impl (const OStoreDirectory_Impl&);
    OStoreDirectory_Impl& operator= (const OStoreDirectory_Impl&);
};

template<> inline OStoreDirectory_Impl*
SAL_CALL query (IStoreHandle *pHandle, OStoreDirectory_Impl*)
{
    if (pHandle && pHandle->isKindOf (OStoreDirectory_Impl::m_nTypeId))
    {
        // Handle is kind of OStoreDirectory_Impl.
        return static_cast<OStoreDirectory_Impl*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_STORDIR_HXX_ */

