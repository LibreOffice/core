#!/bin/sh

./status.sh | xmllint --format - | xsltproc todo.xsl - | xsltproc effort.xsl -
