# Needed just to satisfy ndk-gdb for now, but maybe later we will actually add
# some JNI code here

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include $(BUILD_SHARED_LIBRARY)
