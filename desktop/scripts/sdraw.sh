#!/bin/sh

cmd=$(dirname "$0")/soffice
exec "$cmd" --draw "$@"
