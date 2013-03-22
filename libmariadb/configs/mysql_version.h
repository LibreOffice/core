/* Copyright Abandoned 1996, 1999, 2001 MySQL AB
   This file is public domain and comes with NO WARRANTY of any kind */

/* Version numbers for protocol & mysqld */

#ifdef _CUSTOMCONFIG_
#include <custom_conf.h>
#else
#define PROTOCOL_VERSION 10
#define MYSQL_CLIENT_VERSION "5.3.2"
#define MYSQL_SERVER_VERSION "5.3.2"
#define MYSQL_SERVER_SUFFIX ""
#define FRM_VER
#define MYSQL_VERSION_ID 50302
#define MYSQL_PORT 3306
#define MYSQL_UNIX_ADDR "/tmp/mysql.sock"
#define MYSQL_CONFIG_NAME "my"

/* mysqld compile time options */
#ifndef MYSQL_CHARSET
#define MYSQL_CHARSET ""
#endif
#endif

