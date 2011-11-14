/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http: *www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
 * Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <jni.h>
#include <linux/elf.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "lo-bootstrap", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "lo-bootstrap", __VA_ARGS__))

static char *
read_section(JNIEnv *env,
             int fd,
             Elf32_Shdr *shdr)
{
    char *result;

    result = malloc(shdr->sh_size);
    if (lseek(fd, shdr->sh_offset, SEEK_SET) < 0) {
        close(fd);
        free(result);
        return NULL;
    }
    if (read(fd, result, shdr->sh_size) < shdr->sh_size) {
        close(fd);
        free(result);
        return NULL;
    }

    return result;
}

jobjectArray
Java_org_libreoffice_android_Bootstrap_dlneeds(JNIEnv* env,
                                               jobject clazz,
                                               jstring library)
{
    int i, fd;
    int n_needed;
    const jbyte *libName;
    jobjectArray result;
    char *shstrtab, *dynstr;
    Elf32_Ehdr hdr;
    Elf32_Shdr shdr;
    Elf32_Dyn dyn;

    /* Open library and read ELF header */

    libName = (*env)->GetStringUTFChars(env, library, NULL);

    LOGI("dlneeds(%s)\n", libName);

    fd = open (libName, O_RDONLY);

    (*env)->ReleaseStringUTFChars(env, library, libName);

    if (fd == -1) {
        LOGI("Could not open library");
        return NULL;
    }

    if (read(fd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        close(fd);
        LOGI("Could not read ELF header");
        return NULL;
    }

    /* Read in .shstrtab */

    if (lseek(fd, hdr.e_shoff + hdr.e_shstrndx * sizeof(shdr), SEEK_SET) < 0) {
        close(fd);
        LOGI("Could not seek to .shstrtab section header");
        return NULL;
    }
    if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
        close(fd);
        LOGI("Could not read section header");
        return NULL;
    }

    shstrtab = read_section(env, fd, &shdr);
    if (shstrtab == NULL)
        return NULL;

    /* Read section headers, looking for .dynstr section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        close(fd);
        LOGI("Could not seek to section headers");
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
            close(fd);
            LOGI("Could not read section header");
            return NULL;
        }
        if (shdr.sh_type == SHT_STRTAB &&
            strcmp(shstrtab + shdr.sh_name, ".dynstr") == 0) {
            dynstr = read_section(env, fd, &shdr);
            if (dynstr == NULL) {
                free(shstrtab);
                return NULL;
            }
            break;
        }
    }

    if (i == hdr.e_shnum) {
        close(fd);
        LOGI("No .dynstr section");
        return NULL;
    }

    /* Read section headers, looking for .dynamic section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        close(fd);
        LOGI("Could not seek to section headers");
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
            close(fd);
            LOGI("Could not read section header");
            return NULL;
        }
        if (shdr.sh_type == SHT_DYNAMIC) {
            int dynoff;
            int *libnames;
            jclass String;

            /* Count number of DT_NEEDED entries */
            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                close(fd);
                LOGI("Could not seek to .dynamic section");
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < sizeof(dyn)) {
                    close(fd);
                    LOGI("Could not read .dynamic entry");
                    return NULL;
                }
                if (dyn.d_tag == DT_NEEDED)
                    n_needed++;
            }

            LOGI("Found %d DT_NEEDED libs", n_needed);

            /* Allocate return value */

            String = (*env)->FindClass(env, "java/lang/String");
            if (String == NULL) {
                close(fd);
                LOGI("Could not find the String class");
                return NULL;
            }

            result = (*env)->NewObjectArray(env, n_needed, String, NULL);
            if (result == NULL) {
                close (fd);
                LOGI("Could not create the String array");
                return NULL;
            }

            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                close(fd);
                LOGI("Could not seek to .dynamic section");
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < sizeof(dyn)) {
                    close(fd);
                    LOGI("Could not read .dynamic entry");
                    return NULL;
                }
                if (dyn.d_tag == DT_NEEDED) {
                    LOGI("needs: %s\n", dynstr + dyn.d_un.d_val);
                    (*env)->SetObjectArrayElement(env, result, n_needed, (*env)->NewStringUTF(env, dynstr + dyn.d_un.d_val));
                    n_needed++;
                }
            }

            close(fd);
            free(dynstr);
            free(shstrtab);
            return result;
        }
    }

    return NULL;
}

jint
Java_org_libreoffice_android_Bootstrap_dlopen(JNIEnv* env,
                                              jobject clazz,
                                              jstring library)
{
    const jbyte *libName = (*env)->GetStringUTFChars(env, library, NULL);
    void *p = dlopen (libName, RTLD_LOCAL);
    LOGI("dlopen(%s) = %p", libName, p);
    (*env)->ReleaseStringUTFChars(env, library, libName);
    if (p == NULL) {
        LOGI(dlerror());
        return 0;
    }
    return (jint) p;
}

jint
Java_org_libreoffice_android_Bootstrap_dlsym(JNIEnv* env,
                                             jobject clazz,
                                             jint handle,
                                             jstring symbol)
{
    const jbyte *symName = (*env)->GetStringUTFChars(env, symbol, NULL);
    void *p = dlsym ((void *)handle, symName);
    LOGI("dlsym(%p,%s) = %p", handle, symName, p);
    (*env)->ReleaseStringUTFChars(env, symbol, symName);
    if (p == NULL) {
        LOGI(dlerror());
        return 0;
    }
    return (jint) p;
}

jint
Java_org_libreoffice_android_Bootstrap_dlcall(JNIEnv* env,
                                              jobject clazz,
                                              jint function,
                                              jobject argument)
{
    jclass StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");

    if (StringArray == NULL) {
        LOGI("Could not find String[] class");
        return 0;
    }

    if ((*env)->IsInstanceOf(env, argument, StringArray)) {
        LOGI("Yes, a string array argument of length %d", (*env)->GetArrayLength(env, argument));
        int argc = (*env)->GetArrayLength(env, argument);
        const char **argv = malloc(sizeof(char *) * (argc+1));
        int i, result;
        for (i = 0; i < argc; i++) {
            argv[i] = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, argument, i), NULL);
            LOGI("argv[%d] = %s", i, argv[i]);
        }
        argv[argc] = NULL;

        int (*fp)(int, const char **) = function;

        result = fp(argc, argv);

        for (i = 0; i < argc; i++)
            (*env)->ReleaseStringUTFChars(env, (*env)->GetObjectArrayElement(env, argument, i), argv[i]);

        free(argv);
        return result;
    }

    /* To be implemented */
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
