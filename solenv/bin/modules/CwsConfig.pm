#*************************************************************************
#
#   $RCSfile: CwsConfig.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-06-26 00:20:18 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************


#
# CwsConfig.pm - package for read CWS config data
#

package CwsConfig;
use strict;

use Carp;
use Config::Tiny;
use URI::Escape;

##### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{_CONFIG_FILE}        = undef;    # config file
    $self->{VCSID}               = undef;    # VCSID
    $self->{CWS_DB_URL_LIST_REF} = undef;    # list of CWS DB servers
    $self->{NET_PROXY}           = undef;    # network proxy
    $self->{CWS_SERVER_ROOT}     = undef;    # cvs server
    $self->{CWS_MIRROR_ROOT}     = undef;    # mirror of cvs server
    $self->{CWS_LOCAL_ROOT}      = undef;    # local cvs server
    bless ($self, $class);
    return $self;
}

sub vcsid
{
    my $self = shift;

    if ( !defined($self->{VCSID}) ) {
        # environment overrides config file
        my $vcsid = $ENV{VCSID};
        if ( !defined($vcsid) ) {
            # check config file
            my $config_file = $self->get_config_file();
            $vcsid = $config_file->{CWS_CONFIG}->{'CVS_ID'};
            if ( !defined($vcsid) ) {
                # give up
                croak("ERROR: no CVS_ID entry found in '\$HOME/.cwsrc'.\n" );
            }
        }
        $self->{VCSID} = $vcsid;
    }
    return $self->{VCSID};
}

sub cws_db_url_list_ref
{
    my $self = shift;

    if ( !defined($self->{CWS_DB_URL_LIST_REF}) ) {
        my $config_file = $self->get_config_file();

        my $i = 1;
        my @cws_db_servers;

        while ( 1 ) {
            my $val = $config_file->{CWS_CONFIG}->{"CWS_DB_SERVER_$i"};
            last if !defined($val);
            push(@cws_db_servers, $val);
            $i++;
        }

        if ( !@cws_db_servers) {
            croak("ERROR: no CWS_DB_SERVER_* entry found in '\$HOME/.cwsrc'.\n" );
        }

        if ( $cws_db_servers[0] =~ /^https:\/\// ) {
            my $id = $self->vcsid();
            my $password = $config_file->{CWS_CONFIG}->{'CVS_PASSWORD'};

            if ( !defined($password) ) {
                croak("ERROR: no CVS_PASSWORD entry found in '\$HOME/.cwsrc'.\n" );
            }

            # We are going to stuff $id and $password in an URL, do proper escaping.
            $id = uri_escape($id);
            $password = uri_escape($password);

            foreach ( @cws_db_servers ) {
                s/^https:\/\//https:\/\/$id:$password@/;
            }
        }

        $self->{CWS_DB_URL_LIST_REF} = \@cws_db_servers;
    }
    return $self->{CWS_DB_URL_LIST_REF};
}

sub net_proxy
{
    my $self = shift;

    if ( !defined($self->{NET_PROXY}) ) {
        my $config_file = $self->get_config_file();
        my $net_proxy = $config_file->{CWS_CONFIG}->{'PROXY'};
        if ( !defined($net_proxy) ) {
            $net_proxy = "";
        }
        $self->{NET_PROXY} = $net_proxy;
    }
    return $self->{NET_PROXY} ? $self->{NET_PROXY} : undef;
}

sub cvs_binary
{
    my $self = shift;

    if ( !defined($self->{CVS_BINARY}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_binary = $config_file->{CWS_CONFIG}->{'CVS_BINARY'};
        if ( !defined($cvs_binary) ) {
            # defaults
            $cvs_binary = ($^O eq 'MSWin32') ? 'cvs.exe' : 'cvs';
        }
        # special case ... don't ask ...
        if ($cvs_binary =~ /cvs.clt2/ && $^O eq 'MSWin32') {
            $cvs_binary = 'cvsclt2.exe';
        }
        $self->{CVS_BINARY} = $cvs_binary;
    }
    return $self->{CVS_BINARY};
}

sub cvs_server_root
{
    my $self = shift;

    if ( !defined($self->{CVS_SERVER_ROOT}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_server_root = $config_file->{CWS_CONFIG}->{'CVS_SERVER_ROOT'};
        if ( !defined($cvs_server_root) ) {
            # give up, this is a mandatory entry
            croak("ERROR: can't parse CVS_SERVER_ROOT entry in '\$HOME/.cwsrc'.\n");
        }
        $self->{CVS_SERVER_ROOT} = $cvs_server_root;
    }
    return $self->{CVS_SERVER_ROOT};
}

sub cvs_mirror_root
{
    my $self = shift;

    if ( !defined($self->{CVS_MIRROR_ROOT}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_mirror_root = $config_file->{CWS_CONFIG}->{'CVS_MIRROR_ROOT'};
        if ( !defined($cvs_mirror_root) ) {
            $cvs_mirror_root = "";
        }
        $self->{CVS_MIRROR_ROOT} = $cvs_mirror_root;
    }
    return $self->{CVS_MIRROR_ROOT} ? $self->{CVS_MIRROR_ROOT} : undef;
}

sub cvs_local_root
{
    my $self = shift;

    if ( !defined($self->{CVS_LOCAL_ROOT}) ) {
        my $config_file = $self->get_config_file();
        my $cvs_local_root = $config_file->{CWS_CONFIG}->{'CVS_LOCAL_ROOT'};
        if ( !defined($cvs_local_root) ) {
            $cvs_local_root = "";
        }
        $self->{CVS_LOCAL_ROOT} = $cvs_local_root;
    }
    return $self->{CVS_LOCAL_ROOT} ? $self->{CVS_LOCAL_ROOT} : undef;
}

sub get_cvs_server
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $server;
}

sub get_cvs_mirror
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $server;
}

sub get_cvs_local
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $server;
}

sub get_cvs_server_method
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $method;
}

sub get_cvs_mirror_method
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $method;
}

sub get_cvs_local_method
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $method;
}

sub get_cvs_server_repository
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $repository;
}

sub get_cvs_mirror_repository
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $repository;
}

sub get_cvs_local_repository
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $repository;
}

sub get_cvs_server_id
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_server_root(), 'SERVER');
    return $id;
}

sub get_cvs_mirror_id
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_mirror_root(), 'MIRROR');
    return $id;
}

sub get_cvs_local_id
{
    my $self = shift;

    my ($method, $id, $server, $repository) = CwsConfig::split_root($self->cvs_local_root(), 'LOCAL');
    return $id;
}

#### class methods #####
sub get_config
{
    my $config = CwsConfig->new();
    return $config;
}

sub split_root
{
    my $root = shift;
    my $type = shift;

    if ( !defined($root) ) {
        return (undef, undef, undef, undef);
    }

    my ($dummy, $method, $id_at_host, $repository) = split(/:/, $root);
    my ($id, $server);
    if ( $id_at_host ) {
        ($id, $server) = split(/@/, $id_at_host);
    }
    if ( !defined($method) || !defined($id) || !defined($server) || !defined($repository) ) {
        # give up
        print  "$method, $id, $server, $repository\n";
        croak("ERROR: can't parse CVS_".$type."_ROOT entry in '\$HOME/.cwsrc'.\n");
    }
    return ($method, $id, $server, $repository);
}

#### private helper methods ####

sub get_config_file
{
    my $self = shift;

    if ( !defined $self->{_CONFIG_FILE} ) {
        $self->parse_config_file();
    }
    return $self->{_CONFIG_FILE};
}

sub parse_config_file
{
    my $self = shift;

    my $config_file;
    # check for config files
    if ( -e "$ENV{HOME}/.cwsrc" ) {
        $config_file = Config::Tiny->read("$ENV{HOME}/.cwsrc");
    }
    elsif ( -e "$ENV{COMMON_ENV_TOOLS}/cwsrc" ) {
        $config_file = Config::Tiny->read("$ENV{COMMON_ENV_TOOLS}/cwsrc");
    }
    else {
        croak("ERROR: can't find CWS config file '\$HOME/.cwsrc'.\n");
    }

    croak("ERROR: can't read CWS config file '\$HOME/.cwsrc'.\n") if !defined($config_file);

    $self->{_CONFIG_FILE}=$config_file;
}

1; # needed by "use" or "require"
