workspace(name = "libreoffice")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_googlesource_gerrit_bazlets",
    commit = "fbe2b2fd07c95d752dced6b8624c9d5a08e8c6c6",
    remote = "https://gerrit.googlesource.com/bazlets",
)

# Comment out the git_repository rule above to consume
# Bazlets from local repository
#local_repository(
#    name = "com_googlesource_gerrit_bazlets",
#    path = "/home/<user>/projects/bazlets",
#)
