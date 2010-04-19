#ifndef INCLUDED_FWEDLLAPI_H
#define INCLUDED_FWEDLLAPI_H

#include "sal/types.h"

#if defined(FWE_DLLIMPLEMENTATION)
#define FWE_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#include <cppuhelper/interfacecontainer.h>
template struct FWE_DLLPUBLIC cppu::OBroadcastHelperVar<class cppu::OMultiTypeInterfaceContainerHelper,class com::sun::star::uno::Type>;
//class FWE_DLLPUBLIC SvPtrarr;
#else
#define FWE_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define FWE_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_FWEDLLAPI_H */
