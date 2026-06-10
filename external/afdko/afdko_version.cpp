/*
 * AFDKO Version Implementation
 *
 * Static replacement for the cmake-generated afdko_version.cpp. Keep the
 * version here in sync with AFDKO_TARBALL in download.lst.
 */

#include "afdko_version.h"
#include <string.h>
#include <stdlib.h>

static const char build_version[] = "5.0.1";

/* Mutable version pointer - initialized by init_fdk_version() */
char *FDK_VERSION = NULL;

/* Initialize FDK_VERSION with a malloc'd copy */
void init_fdk_version(void) {
    if (FDK_VERSION == NULL) {
        FDK_VERSION = (char *)malloc(strlen(build_version) + 1);
        if (FDK_VERSION != NULL) {
            strcpy(FDK_VERSION, build_version);
        }
    }
}
