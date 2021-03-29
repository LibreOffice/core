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
