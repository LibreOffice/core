#ifndef INCLUDED_COMPHELPERDLLAPI_H
#define INCLUDED_COMPHELPERDLLAPI_H
                                                                                                                             
#include "sal/types.h"
                                                                                                                             
#if defined(COMPHELPER_DLLIMPLEMENTATION)
#define COMPHELPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define COMPHELPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define COMPHELPER_DLLPRIVATE SAL_DLLPRIVATE
                                                                                                                             
#endif /* INCLUDED_COMPHELPERDLLAPI_H */

