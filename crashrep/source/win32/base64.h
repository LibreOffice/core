#ifndef __BASE64_H
#define __BASE64_H

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_encode( FILE *fin, FILE *fout );

#ifdef __cplusplus
}
#endif

#endif /* __BASE64_H */

