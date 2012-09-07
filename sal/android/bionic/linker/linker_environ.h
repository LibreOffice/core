/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef LINKER_ENVIRON_H
#define LINKER_ENVIRON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Call this function before anything else. 'vecs' must be the pointer
 * to the environment block in the ELF data block. The function returns
 * the start of the aux vectors after the env block.
 */
extern unsigned*   linker_env_init(unsigned* vecs);

/* Unset a given environment variable. In case the variable is defined
 * multiple times, unset all instances. This modifies the environment
 * block, so any pointer returned by linker_env_get() after this call
 * might become invalid */
extern void        linker_env_unset(const char* name);


/* Returns the value of environment variable 'name' if defined and not
 * empty, or NULL otherwise. Note that the returned pointer may become
 * invalid if linker_env_unset() or linker_env_secure() are called
 * after this function. */
extern const char* linker_env_get(const char* name);

/* Remove insecure environment variables. This should be used when
 * running setuid programs. */
extern void        linker_env_secure(void);

#ifdef __cplusplus
};
#endif

#endif /* LINKER_ENVIRON_H */
