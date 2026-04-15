# Database Connectivity

Contains database pieces, drivers, etc.

`dbaccess` builds UI on top of this.

## Testing
### PostgreSQL

For testing, use:

    podman pull postgres:latest
    podman run --name=postgres -e POSTGRES_PASSWORD=foobarbaz -p 127.0.0.1:5432:5432 postgres:latest

In Base, Connect to an existing database, select PostgreSQL:

    URL: host=127.0.0.1 port=5432 dbname=postgres
    User: postgres
    Password: foobarbaz
    
    podman stop postgres
    podman rm postgres

In order to test SCRAM authentication, create the container like this:

    podman run --name=postgres -e POSTGRES_PASSWORD=foobarbaz -e POSTGRES_INITDB_ARGS=--auth-host=scram-sha-256 -e POSTGRES_HOST_AUTH_METHOD=scram-sha-256 -p 127.0.0.1:5432:5432 postgres:latest

### MySQL

For mysql_test:

- The CppunitTest_mysql_test unit test can be used to test the mysqlc
library with any versions of mysql or mariadb server of your choice.
  
- This test does not run automatically. It can be triggered with setting
the environment variable "CONNECTIVITY_TEST_MYSQL_DRIVER".
  
- The environment variable should contain a URL of the following format:
`[user]/[passwd]@sdbc:mysql:mysqlc:[host]:[port]/db_name`

- tl;dr:

```
    podman pull mariadb/server
    podman run --name=mariadb -e MYSQL_ROOT_PASSWORD=foobarbaz -p 127.0.0.1:3306:3306 mariadb/server
    podman exec -it mariadb /bin/bash -c "echo -e CREATE DATABASE test | /usr/bin/mysql -u root"
    (cd connectivity && make -srj8 CppunitTest_connectivity_mysql_test CONNECTIVITY_TEST_MYSQL_DRIVER="root/foobarbaz@sdbc:mysql:mysqlc:127.0.0.1:3306/test")
    podman stop mariadb
    podman rm mariadb
```

### Firebird

Firebird has two primary file types:

- Databases - FDB files. These are version-specific, platform-specific, optimized for performance, and thus incompatible between versions. These are what those comments are about. Initially, when FB integration was considered, these files were evaluated for ODBs, but were rejected because of the said incompatibility - even when the version is the same, it will differ on big endian architecture and little endian one. The problem discussed in those comments is when people open stand-alone FDBs that are shipped e.g. with FB installation itself, not when people open ODBs.

- Database backups - FBKs. These are what we use inside ODBs. These are designed to be compatible, independent of architecture; and later versions of FB are always able to open FBKs created in older FB versions.

Our embedded FB is used like this:
- FBK is extracted from ODB;
- Embedded FB extracts the compatible FBK into an incompatible FDB (specific to this version of embedded FB DLL);
- FB works with this temporary FDB;
- When saving ODB, embedded FB backups the FDB into FBK again, and that is stored inside the ODB.

It, indeed, creates additional performance penalty, but makes the ODB readable by all the future LO versions, no matter what future FB version they embed.
